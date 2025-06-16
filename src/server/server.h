#pragma once
#include <netinet/in.h>
#include <sys/epoll.h>

#include <queue>
#include <unordered_map>
#include <vector>
// #include "../utils/net/net.h"
#include "../utils/query.h"
#include "../query_engine/query_engine.h"


#include "utils-server.h"
#include "TS-queue.h"
void make_non_blocking(int32_t sock);
class EpollServer {
 public:
  EpollServer(int32_t port, int32_t num_worker_threads);
  ~EpollServer();

  void run();

 private:
  sockaddr_in server_address_;
  int32_t server_listen_fd_;
  int32_t epoll_fd_;
  void add_to_epoll(int32_t sock, uint32_t events);
  void bind_server();
  int32_t handle_trade_data_query(int32_t sock, TradeDataQuery query);
  std::queue<std::pair<int32_t, TradeDataQuery>> task_queue_;

  std::vector<std::thread> worker_threads_;
  TSQueue<WorkItem> work_queue_;
  TSQueue<ResultItem> results_queue_;

  void handle_read(int32_t client_fd);
  void handle_write(int32_t client_fd);
  void process_results();
};
