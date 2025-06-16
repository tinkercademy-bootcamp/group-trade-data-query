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
  void serialise_and_enqueue(int id, const std::string& final_result);
  void offload_one();

 private:
  std::queue<std::pair<int, std::string>> queue_;

  std::pair<int, std::string> front() { return queue_.front(); }
  void push(const std::pair<int, std::string>& data) { queue_.push(data); }
  void pop() { queue_.pop(); }
  bool empty() const { return queue_.empty(); }
};

std::string serialise(const std::string& final_result);
void send_data(int sockfd, const std::string& data);
void send_without_serialisation(int sockfd, Result& result, const sockaddr_in& client_addr);
void send_without_serialisation(int sockfd, TradeData& data, const sockaddr_in& client_addr);