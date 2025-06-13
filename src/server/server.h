#pragma once
#include <netinet/in.h>
#include <sys/epoll.h>

#include <queue>

#include "../utils/net/net.h"
#include "../utils/query.h"
#include "../executor/executor.h"

void make_non_blocking(int sock);
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
  void add_to_epoll(int sock);
  void bind_server();
  int handle_trade_data_query(int sock, TradeDataQuery query);
  std::queue<std::pair<int, TradeDataQuery>> task_queue_;
};
