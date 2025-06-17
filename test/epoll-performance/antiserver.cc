#include "antiserver.h"
#include "../nlohmann/json.hpp"
#include <arpa/inet.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

using nlohmann::json;

AntiServer::AntiServer(std::string_view host, uint16_t port, int n_clients)
    : host_{host}, port_{port}, epoll_fd_{epoll_create1(0)} {
  std::cout << "[AntiServer] Constructor called with host: " << host << ", port: " << port << ", n_clients: " << n_clients << std::endl;
  if (epoll_fd_ < 0)
    throw std::runtime_error("epoll_create1 failed");
  clients_.resize(n_clients);
  std::cout << "[AntiServer] Resized clients_ to " << n_clients << std::endl;
  prepare_clients();
  load_testcases("test/performance/performance_tests.json");
}

AntiServer::~AntiServer() {
  std::cout << "[AntiServer] Destructor called" << std::endl;
  for (auto &c : clients_) {
    if (c.fd >= 0) {
      std::cout << "[AntiServer] Closing client fd: " << c.fd << std::endl;
      ::close(c.fd);
    }
  }
  if (epoll_fd_ >= 0) {
    std::cout << "[AntiServer] Closing epoll fd: " << epoll_fd_ << std::endl;
    ::close(epoll_fd_);
  }
}

void AntiServer::run() {
  std::cout << "[AntiServer] run() called" << std::endl;
  while (true) {
    static size_t count = 0;
    if ((++count & 0xFF) == 0) {
      std::cout << "[AntiServer] processing epoll events, iteration: " << count
                << std::endl;
    }
    process_epoll_events();
  }
}

void AntiServer::load_testcases(const std::string &path) {
  std::cout << "[AntiServer] Loading testcases from: " << path << std::endl;
  std::ifstream in(path);
  if (!in)
    throw std::runtime_error("Cannot open testcase file: " + path);

  nlohmann::json root;
  in >> root;

  auto push_case = [this](const nlohmann::json &obj) {
    std::cout << "[AntiServer] Pushing test case: " << obj.value("test_id", "") << std::endl;
    cases_.push_back(TestCase{obj.value("test_id", ""), obj.value("input", ""),
                              obj.value("expected_output", ""),
                              obj.value("error_msg", "")});
  };
  if (root.is_array()) {
    std::cout << "[AntiServer] Testcase JSON is array" << std::endl;
    for (const auto &obj : root)
      push_case(obj);
  } else if (root.is_object() && root.contains("cases") &&
             root["cases"].is_array()) {
    std::cout << "[AntiServer] Testcase JSON is object with 'cases' array" << std::endl;
    for (const auto &obj : root["cases"])
      push_case(obj);
  } else if (root.is_object()) {
    std::cout << "[AntiServer] Testcase JSON is single object" << std::endl;
    push_case(root);
  } else {
    std::cerr << "[AntiServer] Unrecognised testcase JSON layout" << std::endl;
    throw std::runtime_error("Unrecognised testcase JSON layout");
  }
  std::cout << "[AntiServer] Loaded " << cases_.size() << " test cases from "
            << path << std::endl;
}

void AntiServer::prepare_clients() {
  std::cout << "[AntiServer] prepare_clients() called" << std::endl;
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);
  if (inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) != 1) {
    std::cerr << "[AntiServer] inet_pton failed for host: " << host_ << std::endl;
    throw std::runtime_error("inet_pton failed");
  }

  for (size_t i = 0; i < clients_.size(); ++i) {
    std::cout << "[AntiServer] Preparing client " << i << std::endl;
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
    std::cout << "[AntiServer] Client " << i << " got fd " << fd << std::endl;
    add_fd_to_epoll(fd);
    fd_to_idx_[fd] = i;
    std::cout << "[AntiServer] Prepared client " << i << " (fd=" << fd << ")\n";
  }
}

void AntiServer::process_epoll_events() {
  std::cout << "[AntiServer] process_epoll_events() called" << std::endl;
  constexpr int MAX = 64;
  epoll_event events[MAX];
  int nfds = ::epoll_wait(epoll_fd_, events, MAX, 1000);
  if (nfds < 0) {
    perror("[AntiServer] epoll_wait");
    return;
  }
  std::cout << "[AntiServer] epoll_wait returned " << nfds << " events" << std::endl;

  for (int n = 0; n < nfds; ++n) {
    int fd = events[n].data.fd;
    auto it = fd_to_idx_.find(fd);
    if (it == fd_to_idx_.end()) {
      std::cerr << "[AntiServer] fd " << fd << " not found in fd_to_idx_" << std::endl;
      continue;
    }
    ClientState &c = clients_[it->second];

    if (events[n].events & (EPOLLERR | EPOLLHUP)) {
      std::cerr << "[AntiServer] fd " << fd << " closed (EPOLLERR|EPOLLHUP)" << std::endl;
      ::close(fd);
      c.fd = -1;
      continue;
    }

    if ((events[n].events & EPOLLOUT) && !c.awaiting_reply) {
      std::cout << "[AntiServer] fd " << fd << " is writable and not awaiting reply" << std::endl;
      const TestCase &tc = cases_[it->second % cases_.size()];
      send_next_case(c, tc);
    }
    if (events[n].events & EPOLLIN) {
      std::cout << "[AntiServer] fd " << fd << " is readable" << std::endl;
      handle_readable(c);
    }
  }
}

void AntiServer::add_fd_to_epoll(int fd) {
  std::cout << "[AntiServer] Adding fd " << fd << " to epoll" << std::endl;
  epoll_event ev{};
  ev.events = EPOLLIN | EPOLLOUT;
  ev.data.fd = fd;
  if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    std::cerr << "[AntiServer] epoll_ctl ADD failed for fd " << fd << std::endl;
    throw std::runtime_error("epoll_ctl ADD failed");
  }
}

void AntiServer::send_next_case(ClientState &c, const TestCase &tc) {
  std::cout << "[AntiServer] Sending test case: " << tc.id << " to fd " << c.fd << std::endl;
  ssize_t sent = ::send(c.fd, tc.request.data(), tc.request.size(), 0);
  if (sent != static_cast<ssize_t>(tc.request.size())) {
    perror("[AntiServer] send");
    std::cerr << "[AntiServer] Failed to send full request for test case: " << tc.id << std::endl;
    return;
  }
  std::cout << "[AntiServer] Sent test case: " << tc.id << " to fd " << c.fd << std::endl;
  c.awaiting_reply = true;
  c.send_ts = std::chrono::steady_clock::now();
  c.expected_reply = tc.expected_reply;
  c.error_msg = tc.error_msg;
}

void AntiServer::handle_readable(ClientState &c) {
  std::cout << "[AntiServer] handle_readable() called for fd " << c.fd << std::endl;
  char buf[4096];
  ssize_t n = ::recv(c.fd, buf, sizeof(buf), 0);
  if (n <= 0) {
    std::cerr << "[AntiServer] recv returned " << n << " for fd " << c.fd << std::endl;
    return;
  }

  std::string_view reply(buf, static_cast<size_t>(n));
  auto now = std::chrono::steady_clock::now();
  c.latency = now - c.send_ts;
  c.awaiting_reply = false;

  std::cout << "[AntiServer] Received reply: " << reply << " from fd " << c.fd << std::endl;
  if (reply != c.expected_reply) {
    const TestCase &tc = cases_[&c - clients_.data()];
    std::cerr << "[AntiServer] Reply mismatch for test case: " << tc.id << std::endl;
    std::cerr << (tc.error_msg.empty() ? "Mismatch!" : tc.error_msg) << '\n';
  }
  report_results();
}

void AntiServer::report_results() const {
  static size_t counter = 0;
  if ((++counter & 0x3F) != 0)
    return;

  std::cout << "[AntiServer] report_results() called" << std::endl;
  std::chrono::nanoseconds total{};
  size_t ok = 0;
  for (auto const &c : clients_) {
    if (c.latency.count())
      total += c.latency, ++ok;
  }
  if (ok == 0) {
    std::cout << "[AntiServer] No replies to report" << std::endl;
    return;
  }
  double avg_us = total.count() / 1'000.0 / ok;
  std::cout << "[AntiServer] processed " << ok
            << " replies. Avg latency = " << avg_us << " µs\n";
}
