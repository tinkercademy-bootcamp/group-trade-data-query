#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

#include "../utils/query.h"

class OffloadQueue {
public:
  void serialise_and_enqueue(int32_t id, const std::string& final_result);
  void offload_one();

private:
  std::queue<std::pair<int32_t, std::string>> queue_;

  std::pair<int32_t, std::string> front() { return queue_.front(); }
  void push(const std::pair<int32_t, std::string>& data) { queue_.push(data); }
  void pop() { queue_.pop(); }
  bool empty() const { return queue_.empty(); }
};

std::string serialise(const std::string& final_result);
void send_data(int32_t sockfd, const std::string& data);
void send_without_serialisation(int32_t sockfd, Result& result);
void send_without_serialisation(int32_t sockfd, TradeData& data);
