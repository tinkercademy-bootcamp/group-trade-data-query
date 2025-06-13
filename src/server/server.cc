#include "server.h"
#include "../utils/helper/utils.h"
#include "../utils/net/net.h"

EpollServer::EpollServer(int port)
    : server_listen_fd_(net::create_socket()),
      server_address_(net::create_address(port)),
      epoll_fd_(epoll_fd_ = epoll_create1(0)) {
  make_non_blocking(server_listen_fd_);
  bind_server();
  add_to_epoll(server_listen_fd_);
}

EpollServer::~EpollServer() {
  close(server_listen_fd_);
  close(epoll_fd_);
}

void EpollServer::run() {
  helper::check_error(listen(server_listen_fd_, SOMAXCONN) < 0,
                      "Failed to listen on server socket");
}

void EpollServer::accept_connection() {
  sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_fd =
      accept(server_listen_fd_, reinterpret_cast<sockaddr*>(&client_addr),
             &client_len);
  helper::check_error(client_fd < 0, "Failed to accept connection");
  make_non_blocking(client_fd);
  add_to_epoll(client_fd);
}

void EpollServer::add_to_epoll(int sock) {
  epoll_event event{};
  event.data.fd = sock;
  event.events = EPOLLIN | EPOLLET;  // Edge-triggered, read events

  helper::check_error(
      epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sock, &event) == -1,
      "Failed to add client socket to epoll");
}

void make_non_blocking(int sock) {
  int flags = fcntl(sock, F_GETFL, 0);
  helper::check_error(flags == -1, "Failed to get socket flags");
  helper::check_error(fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1, "Failed to set socket to non-blocking");
}

void EpollServer::bind_server() {
  helper::check_error(
      bind(server_listen_fd_, reinterpret_cast<sockaddr*>(&server_address_),
           sizeof(server_address_)) < 0,
      "Failed to bind server socket");
}