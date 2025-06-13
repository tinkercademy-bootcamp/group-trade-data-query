#include "server.h"
// #include "../utils/helper/utils.h"
#include "../utils/net/net.h"


EpollServer::EpollServer(int port)
    : server_listen_fd_(net::create_socket()),
      server_address_(net::create_address(port)) {
        make_non_blocking();
        bind_server();
        add_to_epoll();
      }

EpollServer::~EpollServer() {}

void EpollServer::run() {}

void EpollServer::accept_connection() {}

void EpollServer::add_to_epoll() {
  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1) {
    throw std::runtime_error("Failed to create epoll file descriptor: " + std::string(strerror(errno)));
  }

  epoll_event event;
  event.data.fd = server_listen_fd_;
  event.events = EPOLLIN | EPOLLET; // Edge-triggered, read events

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_listen_fd_, &event) == -1) {
    throw std::runtime_error("Failed to add server socket to epoll: " + std::string(strerror(errno)));
  }
}


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
