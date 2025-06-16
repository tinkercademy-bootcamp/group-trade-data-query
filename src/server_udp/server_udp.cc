#include "server_udp.h"
// #include "../../processed_data/preproc.cc"
#include <spdlog/spdlog.h>

#include "../utils/helper/utils.h"
#include "../utils/net_udp/net_udp.h"
#include "../utils/query.h"
#include "sender_udp.h"

/////////////////

std::vector<TradeData> execute_task(TradeDataQuery& query) {
  // Dummy implementation: return an empty vector
  spdlog::info("Query processed for TradeDataQuery id: {}", query.symbol_id);
  return {};
}

/////////////////

constexpr int32_t MAX_EVENTS = 10;

EpollServer::EpollServer(int32_t port)
    : server_listen_fd_(net::create_socket()),
      server_address_(net::create_address(port)),
      epoll_fd_(epoll_create1(0)) {
  int32_t opt = 1;
  helper::check_error(setsockopt(server_listen_fd_, SOL_SOCKET, SO_REUSEADDR,
                                 &opt, sizeof(opt)) < 0,
                      "Failed to set SO_REUSEADDR");
  helper::check_error(setsockopt(server_listen_fd_, SOL_SOCKET, SO_REUSEPORT,
                                 &opt, sizeof(opt)) < 0,
                      "Failed to set SO_REUSEPORT");
  make_non_blocking(server_listen_fd_);
  bind_server();
  add_to_epoll(server_listen_fd_);
}

EpollServer::~EpollServer() {
  close(server_listen_fd_);
  close(epoll_fd_);
  spdlog::info("Server ended");
}

void EpollServer::run() {
  epoll_event events[MAX_EVENTS];
  while (true) {
    int32_t n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
    helper::check_error(n == -1, "epoll_wait failed");

    for (int32_t i = 0; i < n; ++i) {
      if (events[i].data.fd == server_listen_fd_) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        TradeDataQuery query;
        ssize_t count =
            recvfrom(server_listen_fd_, &query, sizeof(query), 0,
                     reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (count < 0) {
          spdlog::error("recvfrom failed: {}", strerror(errno));
          continue;
        } else {
          char client_ip[INET_ADDRSTRLEN];
          inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip,
                    INET_ADDRSTRLEN);
          int32_t client_port = ntohs(client_addr.sin_port);

          spdlog::info("Processing query from client fd {}",
                       static_cast<int32_t>(events[i].data.fd));
          helper::check_error(handle_trade_data_query(events[i].data.fd, query,
                                                      client_addr) < 0,
                              "Failed to handle the query");
        }
      }
    }
  }
}

int32_t EpollServer::handle_trade_data_query(int32_t client_sock, TradeDataQuery query,
                                             sockaddr_in client_addr) {
  std::vector<Result> rresult;
  std::vector<TradeData> tresult;
  Executor exec;
  int32_t result_size;
  bool t_not_r;
  if (query.resolution > 0) {
    rresult = exec.lowest_and_highest_prices(query);
    result_size = static_cast<int32_t>(rresult.size());
    t_not_r = false;
  } else {
    tresult = exec.send_raw_data(query);
    result_size = static_cast<int32_t>(tresult.size());
    t_not_r = true;
  }
  // First, send the size of the result vector
  ssize_t bytes_sent =
      sendto(client_sock, &result_size, sizeof(result_size), 0,
             reinterpret_cast<sockaddr*>(&client_addr), sizeof(client_addr));
  if (bytes_sent < 0) {
    throw std::runtime_error("Failed to send result size: " +
                             std::string(strerror(errno)));
  } else if (bytes_sent < static_cast<ssize_t>(sizeof(result_size))) {
    std::cerr << "Warning: Only " << bytes_sent << " out of "
              << sizeof(result_size) << " bytes sent for result size."
              << std::endl;
  }

  if (t_not_r) {
    for (auto& data : tresult) {
      send_without_serialisation(client_sock, data, client_addr);
    }
  } else {
    for (auto& data : rresult) {
      send_without_serialisation(client_sock, data, client_addr);
    }
  }
  return 0;
}

void EpollServer::add_to_epoll(int32_t sock) {
  epoll_event event{};
  event.data.fd = sock;
  event.events = EPOLLIN | EPOLLET;  // Edge-triggered, read events

  helper::check_error(epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sock, &event) == -1,
                      "Failed to add socket to epoll");
  spdlog::info("Socket {} added to epoll.", sock);
}

void make_non_blocking(int32_t sock) {
  int32_t flags = fcntl(sock, F_GETFL, 0);
  helper::check_error(flags == -1, "Failed to get socket flags");
  helper::check_error(fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1,
                      "Failed to set socket to non-blocking");
  spdlog::info("Server switched socket {} to non-blocking mode.", sock);
}

void EpollServer::bind_server() {
  helper::check_error(
      bind(server_listen_fd_, reinterpret_cast<sockaddr*>(&server_address_),
           sizeof(server_address_)) < 0,
      "Failed to bind server socket");
  spdlog::info("Server bound to port {}", ntohs(server_address_.sin_port));
}