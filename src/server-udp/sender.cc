#include "sender.h"

#include "../utils/query.h"

void OffloadQueue::serialise_and_enqueue(int sockfd,
                                         const std::string& final_result) {
  std::string serialised_data = serialise(final_result);
  queue_.push({sockfd, serialised_data});
}

void OffloadQueue::offload_one() {
  if (!queue_.empty()) {
    auto [sockfd, data] = queue_.front();
    send_data(sockfd, data);
    queue_.pop();
  }
}

void send_data(int sockfd, const std::string& data) {
  ssize_t bytes_sent = send(sockfd, data.c_str(), data.size(), 0);
  if (bytes_sent < 0) {
    throw std::runtime_error("Failed to send data: " +
                             std::string(strerror(errno)));
  } else if (bytes_sent < static_cast<ssize_t>(data.size())) {
    std::cerr << "Warning: Only " << bytes_sent << " out of " << data.size()
              << " bytes sent." << std::endl;
  }
}

// not implemented for now
std::string serialise(const std::string& final_result) { return final_result; }

void send_without_serialisation(int sockfd, Result& result) {
  ssize_t bytes_sent = send(sockfd, &result, sizeof(result), 0);
  if (bytes_sent < 0) {
    throw std::runtime_error("Failed to send data: " +
                             std::string(strerror(errno)));
  } else if (bytes_sent < static_cast<ssize_t>(sizeof(result))) {
    std::cerr << "Warning: Only " << bytes_sent << " out of " << sizeof(result)
              << " bytes sent." << std::endl;
  }
}

void send_without_serialisation(int sockfd, TradeData& data) {
  ssize_t bytes_sent = send(sockfd, &data, sizeof(data), 0);
  if (bytes_sent < 0) {
    throw std::runtime_error("Failed to send data: " +
                             std::string(strerror(errno)));
  } else if (bytes_sent < static_cast<ssize_t>(sizeof(data))) {
    std::cerr << "Warning: Only " << bytes_sent << " out of " << sizeof(data)
              << " bytes sent." << std::endl;
  }
}
