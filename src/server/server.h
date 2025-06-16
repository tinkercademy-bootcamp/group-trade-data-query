#pragma once
#include <netinet/in.h>
#include <sys/epoll.h>
#include <memory>

#include <queue>

// #include "../utils/net/net.h"
#include "../utils/query.h"
#include "../query_engine/query_engine.h"
#include "../utils/mt_queue.h"

void make_non_blocking(int32_t sock);
class EpollServer
{
public:
  EpollServer(int32_t port);
  ~EpollServer();

  void run();

private:
  sockaddr_in server_address_;
  int32_t server_listen_fd_;
  int32_t epoll_fd_;
  void accept_connection();
  void add_to_epoll(int32_t sock);
  void bind_server();
  int32_t handle_trade_data_query(int32_t sock, TradeDataQuery query);
  Mt_Queue<std::pair<int32_t, TradeDataQuery>> task_queue_;
  Mt_Queue<std::pair<int32_t, std::vector<Result>>> send_queue_;
  void query_worker();
  void sender_thread();
};
