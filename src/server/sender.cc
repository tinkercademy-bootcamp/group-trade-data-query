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

void send_without_serialisation(int sock, const TradeData& data, const sockaddr_in& client_addr) {
  ssize_t sent = sendto(sock, &data, sizeof(data), 0,
                        reinterpret_cast<const sockaddr*>(&client_addr), sizeof(client_addr));
  if (sent < 0) {
    std::cerr << "Failed to send TradeData: " << strerror(errno) << std::endl;
  }
}

void send_without_serialisation(int sock, const Result& result, const sockaddr_in& client_addr) {
  ssize_t sent = sendto(sock, &result, sizeof(result), 0,
                        reinterpret_cast<const sockaddr*>(&client_addr), sizeof(client_addr));
  if (sent < 0) {
    std::cerr << "Failed to send Result: " << strerror(errno) << std::endl;
  }
}
