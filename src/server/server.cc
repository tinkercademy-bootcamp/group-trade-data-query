#include "server.h"
// #include "../utils/helper/error_handling.h"
#include "../utils/net/socket_helper.h"

EpollServer::EpollServer(int port)
    : server_listen_fd_(net::create_socket()),
      server_address_(net::create_server_address(port)) {
  // set_non_blocking(server_listen_fd_);
  // opt_bind_listen();
  epoll_fd_ = epoll_create1(0);
  // check_error(epoll_fd_ < 0, "Couldn't make epoll socket");
  // add_to_epoll(server_listen_fd_, EPOLLIN | EPOLLET);
}

EpollServer::~EpollServer() {}

void EpollServer::run() {}

void EpollServer::accept_connection() {}

void EpollServer::add_to_epoll() {}