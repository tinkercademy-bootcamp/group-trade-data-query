#include "server.h"
// #include "../utils/helper/utils.h"
#include "../utils/net/net.h"

EpollServer::EpollServer(int port)
    : server_listen_fd_(net::create_socket()),
      server_address_(net::create_address(port)) {
        make_non_blocking();
      }

EpollServer::~EpollServer() {}

void EpollServer::run() {}

void EpollServer::accept_connection() {}

void EpollServer::add_to_epoll() {}

void EpollServer::make_non_blocking() {
  int flags = fcntl(server_listen_fd_, F_GETFL, 0);
  if (flags != -1) {
    fcntl(server_listen_fd_, F_SETFL, flags | O_NONBLOCK);
  }
}

void EpollServer::bind_server() {
  if (bind(server_listen_fd_, reinterpret_cast<sockaddr*>(&server_address_), sizeof(server_address_)) < 0) {
    throw std::runtime_error("Failed to bind server socket: " + std::string(strerror(errno)));
  }
}