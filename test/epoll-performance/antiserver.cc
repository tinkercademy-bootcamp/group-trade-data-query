#include "antiserver.h"
#include <cstdio>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

AntiServer::AntiServer() {
  epoll_fd_ = epoll_create1(0);
  void prepare_sockets(int num_sockets);
  void setup_listener();
}

void AntiServer::prepare_sockets(int num_sockets) {
  for (int i = 0; i < num_sockets; ++i) {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    sockets_.emplace_back(sockfd, false);

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = sockfd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
      perror("epoll_ctl: add");
      close(sockfd);
      sockets_.pop_back();
    }
  }
}

