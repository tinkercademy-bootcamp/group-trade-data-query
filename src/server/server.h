#pragma once
#include <sys/epoll.h>

class EpollServer {
 public:
  EpollServer(int port);
  ~EpollServer();

  void run();

 private:
  int server_listen_fd_;
  int epoll_fd_;
  void accept_connection();
  void add_to_epoll();
};