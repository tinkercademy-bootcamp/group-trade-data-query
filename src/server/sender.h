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


class OffloadQueue {
 public:
  void push(const std::string& data) { queue_.push(data); }
  bool empty() const { return queue_.empty(); }
  std::string front() const { return queue_.front(); }
  void pop() { queue_.pop(); }
  void serialise_and_enqueue(const std::string& final_result);
  
  private:
  std::queue<std::string> queue_;
};

void send_data(int sockfd, const std::string& data);