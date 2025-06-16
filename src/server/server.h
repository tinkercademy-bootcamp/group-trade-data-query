#pragma once
#include <netinet/in.h>
#include <sys/epoll.h>

#include <queue>

// #include "../utils/net/net.h"
#include "../utils/query.h"
#include "../query_engine/query_engine.h"

#define MAX_THREADS 4

void make_non_blocking(int32_t sock);
class EpollServer {
 public:
  EpollServer(int32_t port);
  ~EpollServer();

  void run();

 private:
  sockaddr_in server_address_;
  int32_t server_listen_fd_;
  int32_t epoll_fd_;

  std::mutex task_queue_mutex_;
  std::queue<TradeDataQuery> task_queue_;
  
  // std::mutex request_queue_mutex_;
  // std::queue<> request_queue_;

  std::vector<std::thread> worker_threads_;

  void accept_connection();
  void add_to_epoll(int32_t sock);
  void bind_server();
  int32_t handle_trade_data_query(int32_t sock, TradeDataQuery query);
  std::queue<std::pair<int32_t, TradeDataQuery>> task_queue_;
};

