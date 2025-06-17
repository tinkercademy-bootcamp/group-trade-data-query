#include "antiserver.h"
#include "../nlohmann/json.hpp"
#include <arpa/inet.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include "../../src/utils/query.h"
#include <unordered_map>
#include <cstring>
#include <algorithm>

using nlohmann::json;

void epoll_mod(int epoll_fd, int fd, uint32_t events) {
  epoll_event ev{};
  ev.events = events;
  ev.data.fd = fd;
  if (::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1) {
    perror("[AntiServer] epoll_ctl MOD failed");
  }
}

AntiServer::AntiServer(std::string_view host, uint16_t port, int n_clients)
    : host_{host}, port_{port}, epoll_fd_{epoll_create1(0)} {
  if (epoll_fd_ < 0)
    throw std::runtime_error("epoll_create1 failed");

  clients_.resize(n_clients);
  prepare_clients();
  load_testcases("test/performance/performance_tests.json");
}

AntiServer::~AntiServer() {
  for (auto &c : clients_) {
    if (c.fd >= 0) {
      ::close(c.fd);
    }
  }
  if (epoll_fd_ >= 0) {
    ::close(epoll_fd_);
  }
}

void AntiServer::run() {
  size_t test_case_idx = 0;
  while (true) {
    process_epoll_events(test_case_idx);
  }
}

void AntiServer::load_testcases(const std::string &path) {
  std::ifstream in(path);
  if (!in)
    throw std::runtime_error("Cannot open testcase file: " + path);

  nlohmann::json root;
  in >> root;

  if (!root.is_object() || !root.contains("tests") ||
      !root["tests"].is_array()) {
    throw std::runtime_error(
        "Testcase JSON must be an object with a 'tests' array");
  }

  for (const auto &obj : root["tests"]) {
    TestCase tc;
    tc.id = obj.value("test_id", "");
    tc.expected_reply = obj.value("expected_output", "");
    tc.error_msg = obj.value("error_msg", "");
    
    // Parse the input string into a TradeDataQuery struct
    std::string input = obj.value("input", "");
    std::istringstream iss(input);
    TradeDataQuery query;
    if (iss >> query.symbol_id >> query.start_time_point >> query.end_time_point >> query.resolution >> query.metrics) {
      // Convert the struct to binary data
      tc.request = std::string(reinterpret_cast<const char*>(&query), sizeof(query));
    } else {
      std::cerr << "[AntiServer] Failed to parse input: " << input << std::endl;
      continue;
    }
    
    cases_.push_back(tc);
  }

  std::cout << "[AntiServer] Loaded " << cases_.size() << " test cases from "
            << path << std::endl;
}

void AntiServer::prepare_clients() {
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);
  if (inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) != 1) {
    throw std::runtime_error("inet_pton failed");
  }

  for (size_t i = 0; i < clients_.size(); ++i) {
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd < 0) {
      perror("[AntiServer] socket");
      continue;
    }

    if (::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) ==
            -1 &&
        errno != EINPROGRESS) {
      perror("[AntiServer] connect");
      ::close(fd);
      continue;
    }

    clients_[i].fd = fd;
    add_fd_to_epoll(fd);
    fd_to_idx_[fd] = i;
    std::cout << "[AntiServer] Prepared client " << i << " (fd=" << fd << ")\n";
  }
}

void AntiServer::process_epoll_events(size_t &test_case_idx) {
  constexpr int MAX = 64;
  epoll_event events[MAX];

  int nfds = ::epoll_wait(epoll_fd_, events, MAX, 1000);
  if (nfds < 0) {
    if (errno == EINTR)
      return;
    perror("[AntiServer] epoll_wait");
    return;
  }

  for (int n = 0; n < nfds; ++n) {
    int fd = events[n].data.fd;
    auto it = fd_to_idx_.find(fd);
    if (it == fd_to_idx_.end()) {

      continue;
    }
    ClientState &c = clients_[it->second];

    if (events[n].events & (EPOLLERR | EPOLLHUP)) {
      std::cerr << "[AntiServer] fd " << fd << " closed (EPOLLERR|EPOLLHUP)"
                << std::endl;
      ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
      ::close(fd);
      c.fd = -1;
      fd_to_idx_.erase(it);
      continue;
    }

    if ((events[n].events & EPOLLOUT) && !c.awaiting_reply) {

      const TestCase &tc = cases_[test_case_idx % cases_.size()];
      send_next_case(c, tc);
      test_case_idx++;
    }

    if (events[n].events & EPOLLIN) {
      handle_readable(c);
    }
  }
}

void AntiServer::add_fd_to_epoll(int fd) {
  epoll_event ev{};

  ev.events = EPOLLIN | EPOLLOUT;
  ev.data.fd = fd;
  if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    throw std::runtime_error("epoll_ctl ADD failed");
  }
}

void AntiServer::send_next_case(ClientState &c, const TestCase &tc) {
  ssize_t sent = ::send(c.fd, tc.request.data(), tc.request.size(), 0);
  if (sent < 0) {

    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return;
    perror("[AntiServer] send");
    return;
  }

  if (sent != static_cast<ssize_t>(tc.request.size())) {

    std::cerr << "[AntiServer] Partial send for test case: " << tc.id
              << std::endl;
  }

  std::cout << "[AntiServer] Sent test case: " << tc.id << " to fd " << c.fd
            << std::endl;
  c.awaiting_reply = true;
  c.send_ts = std::chrono::steady_clock::now();
  c.expected_reply = tc.expected_reply;
  c.error_msg = tc.error_msg;

  epoll_mod(epoll_fd_, c.fd, EPOLLIN);
}

void AntiServer::handle_readable(ClientState &c) {
  // Handle partial reads for non-blocking sockets
  static std::unordered_map<int, std::vector<char>> partial_buffers;
  static std::unordered_map<int, int32_t> expected_counts;
  static std::unordered_map<int, int> read_states; // 0=reading_count, 1=reading_results
  
  int fd = c.fd;
  
  // Initialize state if not present
  if (read_states.find(fd) == read_states.end()) {
    read_states[fd] = 0; // start with reading count
    partial_buffers[fd].clear();
  }
  
  if (read_states[fd] == 0) {
    // Reading the result count
    char count_buf[sizeof(int32_t)];
    ssize_t n = ::recv(fd, count_buf, sizeof(int32_t), 0);
    if (n <= 0) {
      if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return; // Try again later
      // Connection error
      std::cerr << "[AntiServer] recv returned " << n << " for fd " << fd
                << ", server likely closed connection." << std::endl;
      ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
      ::close(fd);
      fd_to_idx_.erase(fd);
      c.fd = -1;
      // Clean up state
      read_states.erase(fd);
      partial_buffers.erase(fd);
      expected_counts.erase(fd);
      return;
    }
    
    if (n == sizeof(int32_t)) {
      // Got full count
      int32_t result_count;
      std::memcpy(&result_count, count_buf, sizeof(int32_t));
      expected_counts[fd] = result_count;
      read_states[fd] = 1; // Move to reading results
      partial_buffers[fd].resize(result_count * sizeof(Result));
      partial_buffers[fd].clear(); // Reset for actual data
    } else {
      // Partial read of count - this shouldn't happen with int32_t but handle it
      return; // Try again later
    }
  }
  
  if (read_states[fd] == 1) {
    // Reading the Result structs
    int32_t result_count = expected_counts[fd];
    size_t total_size = result_count * sizeof(Result);
    size_t current_size = partial_buffers[fd].size();
    
    if (current_size < total_size) {
      // Still need to read more data
      char temp_buf[1024];
      size_t to_read = std::min(sizeof(temp_buf), total_size - current_size);
      ssize_t n = ::recv(fd, temp_buf, to_read, 0);
      
      if (n <= 0) {
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
          return; // Try again later
        std::cerr << "[AntiServer] Failed to read Result data for fd " << fd << std::endl;
        return;
      }
      
      // Append to buffer
      partial_buffers[fd].insert(partial_buffers[fd].end(), temp_buf, temp_buf + n);
    }
    
    // Check if we have all the data
    if (partial_buffers[fd].size() >= total_size) {
      // We have all the data, process it
      std::vector<Result> results(result_count);
      std::memcpy(results.data(), partial_buffers[fd].data(), total_size);
      
      // Convert Results to string format for comparison
      std::ostringstream oss;
      if (result_count == 0) {
        // Empty result
      } else {
        for (int32_t i = 0; i < result_count; i++) {
          if (i > 0) oss << "\n";
          const Result& data = results[i];
          int low_exp = static_cast<int>(data.lowest_price.price_exponent);
          int high_exp = static_cast<int>(data.highest_price.price_exponent);
          
          oss << "Timestamp: " << data.start_time
              << "; Min Price: " << data.lowest_price.price << "e" << low_exp
              << "; Max Price: " << data.highest_price.price << "e" << high_exp;
        }
      }
      
      std::string reply = oss.str();
      auto now = std::chrono::steady_clock::now();
      c.latency = now - c.send_ts;
      c.awaiting_reply = false;

      const size_t client_idx = fd_to_idx_[fd];
      const TestCase &tc = cases_[client_idx % cases_.size()];

      if (reply != c.expected_reply) {
        std::cerr << "[AntiServer] Reply mismatch for test case: " << tc.id
                  << std::endl;
        std::cerr << "Expected: '" << c.expected_reply << "', Got: '" << reply
                  << "'\n";
        std::cerr << (tc.error_msg.empty() ? "Mismatch!" : tc.error_msg) << '\n';
      } else {
        std::cout << "[AntiServer] Correct reply from fd " << fd << " in "
                  << std::chrono::duration_cast<std::chrono::microseconds>(
                         c.latency)
                         .count()
                  << " us" << std::endl;
      }
      report_results();

      // Clean up state for this connection
      read_states.erase(fd);
      partial_buffers.erase(fd);
      expected_counts.erase(fd);
      
      epoll_mod(epoll_fd_, fd, EPOLLIN | EPOLLOUT);
    }
  }
}

void AntiServer::report_results() const {
  static size_t counter = 0;
  if ((++counter % clients_.size()) != 0)
    return;

  std::chrono::nanoseconds total{};
  size_t ok = 0;
  for (auto const &c : clients_) {
    if (c.latency.count())
      total += c.latency, ++ok;
  }
  if (ok == 0) {
    return;
  }

  double avg_us = total.count() / 1'000.0 / ok;
  std::cout << "\n[REPORT] Total processed " << counter
            << " replies. Avg latency (running) = " << avg_us << " µs\n\n";
}

void AntiServer::run_wrapper() {
  size_t test_case_idx = 0;
  while (true) {
    process_epoll_events(test_case_idx);
  }
}
