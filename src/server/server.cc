#include "server.h"
#include "../query_engine/query_engine.h"
// #include "../../processed_data/preproc.cc"
#include "../utils/helper/utils.h"
#include "../utils/net/net.h"
#include "../utils/query.h"
#include "sender.h"
#include <spdlog/spdlog.h>

/////////////////

std::vector<TradeData> execute_task(TradeDataQuery& query) {
  // Dummy implementation: return an empty vector
  spdlog::info("Query processed for TradeDataQuery id: {}", query.symbol_id);
  return {};
}

/////////////////

std::vector<std::pair<std::string, uint16_t>> peer_servers = {
  {"3.109.98.204", 35000},  // Example IP and port
  // {"172.31.14.221", 8081}
};
static int query_counter = 0;
constexpr int FORWARD_EVERY = 2;
int connect_to_server(const char* ip, uint16_t port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  helper::check_error(sock < 0, "Socket creation failed");
  sockaddr_in serv_addr{};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  inet_pton(AF_INET, ip, &serv_addr.sin_addr);

  helper::check_error(connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0,
                      "Connection to peer server failed");
  return sock;
}

constexpr int32_t MAX_EVENTS = 10;

EpollServer::EpollServer(uint16_t port)
    : server_listen_fd_(net::create_socket()),
      server_address_(net::create_address(port)),
      epoll_fd_(epoll_fd_ = epoll_create1(0)) {
  int32_t opt = 1;
  helper::check_error(setsockopt(server_listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0,
            "Failed to set SO_REUSEADDR");
  helper::check_error(setsockopt(server_listen_fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0,
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
  helper::check_error(listen(server_listen_fd_, SOMAXCONN) < 0,
                      "Failed to listen on server socket");
  epoll_event events[MAX_EVENTS];
  while (true) {
    int32_t n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
    helper::check_error(n == -1, "epoll_wait failed");

    for (int32_t i = 0; i < n; ++i) {
      if (events[i].data.fd == server_listen_fd_) {
        // Accept all incoming connections
        while (true) {
          int32_t client_fd = accept(server_listen_fd_, nullptr, nullptr);
          if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            helper::check_error(true, "Failed to accept connection");
          }
          make_non_blocking(client_fd);
          add_to_epoll(client_fd);
        }
      } else if (events[i].events & EPOLLIN) {
        // Read data from client
        TradeDataQuery query;
        while (true) {
          ssize_t count = recv(events[i].data.fd, &query, sizeof(query), 0);
          if (count == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            close(events[i].data.fd);
            break;
          } else if (count == 0) {
            // Connection closed
            spdlog::info("Connection closed by client fd {}", static_cast<int32_t>(events[i].data.fd));
            close(events[i].data.fd);
            break;
          } else {
            spdlog::info("Processing query from client fd {}", static_cast<int32_t>(events[i].data.fd));
            helper::check_error(
                handle_trade_data_query(events[i].data.fd, query) < 0,
                "Failed to handle the query");
          }
        }
      }
    }
  }
}

int32_t EpollServer::handle_trade_data_query(int32_t sock, TradeDataQuery query) {
  task_queue_.push(std::make_pair(sock, query));

  while (!task_queue_.empty()) {
    auto [client_sock, task_query] = task_queue_.front();

    task_queue_.pop();
    std::vector<Result> result;
    int32_t result_size;
    Query_engine exec;
    bool forward_to_peer = (++query_counter % FORWARD_EVERY == 0);
    if (forward_to_peer && !peer_servers.empty()) {
        auto& [peer_ip, peer_port] = peer_servers[query_counter % peer_servers.size()];
        spdlog::info("Forwarding query {} to peer server {}:{}", query_counter, peer_ip, peer_port);
        try {
            int sock_fd = connect_to_server(peer_ip.c_str(), peer_port);
            send(sock_fd, &task_query, sizeof(task_query), 0);
            recv(sock_fd, &result_size, sizeof(result_size), MSG_WAITALL);
            result.resize(result_size);
            recv(sock_fd, result.data(), sizeof(Result) * result_size, MSG_WAITALL);
            close(sock_fd);
            spdlog::info("Received {} results from peer server", result_size);
        } catch (const std::exception& e) {
            spdlog::error("Failed to forward to peer server: {}", e.what());
            result = exec.lowest_and_highest_prices(task_query);
            result_size = static_cast<int32_t>(result.size());
        }
    } else {
        result = exec.lowest_and_highest_prices(task_query);
        result_size = static_cast<int32_t>(result.size());
        spdlog::info("Processing query {} locally", query_counter);
    }

    ssize_t bytes_sent = send(client_sock, &result_size, sizeof(result_size), 0);
    if (bytes_sent < 0) {
      throw std::runtime_error("Failed to send result size: " + std::string(strerror(errno)));
    } else if (bytes_sent < static_cast<ssize_t>(sizeof(result_size))) {
      std::cerr << "Warning: Only " << bytes_sent << " out of "
                << sizeof(result_size) << " bytes sent for result size."
                << std::endl;
    }
    for (auto& data : result) {
      send_without_serialisation(client_sock, data);
    }
  }
  return 0;
}

void EpollServer::accept_connection() {
  sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int32_t client_fd =
      accept(server_listen_fd_, reinterpret_cast<sockaddr*>(&client_addr),
             &client_len);
  helper::check_error(client_fd < 0, "Failed to accept connection");
  make_non_blocking(client_fd);
  add_to_epoll(client_fd);

  char client_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
  int32_t client_port = ntohs(client_addr.sin_port);
  spdlog::info("Accepted connection from {}:{}. Assigned fd: {}", client_ip, client_port, client_fd);
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
