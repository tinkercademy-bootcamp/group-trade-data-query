#pragma once
#include <netinet/in.h>
#include <sys/epoll.h>
#include "../utils/net/net.h"

class EpollServer {
 public:
  EpollServer(int port);
  ~EpollServer();

  void run();

 private:
  sockaddr_in server_address_;
  int server_listen_fd_;
  int epoll_fd_;
  void accept_connection();
  void add_to_epoll();
};
