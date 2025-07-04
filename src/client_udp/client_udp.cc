#include "client_udp.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../utils/helper/utils.h"
#include "../utils/net_udp/net_udp.h"

client::Client::Client(uint16_t port, const std::string &server_address)
    : socket_{net::create_socket()},
      server_address_{create_server_address(server_address, port)} {}

void client::Client::send_message(const TradeDataQuery &message) {
  ssize_t bytes_sent =
      sendto(socket_, &message, sizeof(message), 0,
             reinterpret_cast<const sockaddr *>(&server_address_),
             sizeof(server_address_));
  if (bytes_sent < 0) {
    helper::check_error(true, "Send failed on client socket.");
  }
}

int32_t client::Client::get_socket_fd() const { return socket_; }
client::Client::~Client() { close(socket_); }

sockaddr_in client::Client::create_server_address(const std::string &server_ip,
                                                  uint16_t port) {
  sockaddr_in address = net::create_address(port);
  // Convert the server IP address to a binary format
  auto err_code = inet_pton(AF_INET, server_ip.c_str(), &address.sin_addr);
  helper::check_error(err_code <= 0,
                      "Invalid address/ Address not supported\n");
  return address;
}

template <typename T>
std::vector<T> client::Client::read_struct() {
  int32_t count;
  ssize_t n = recv(socket_, &count, sizeof(count), 0);

  helper::check_error(n < 0, "Failed reading the size.\n");

  std::vector<T> output(count);
  for (int32_t i = 0; i < count; i++) {
    n = recv(socket_, &(output[i]), sizeof(output[i]), 0);
    helper::check_error(n < 0, "Failed reading a Result struct.\n");
  }

  return output;
}

template std::vector<TradeData> client::Client::read_struct<TradeData>();
template std::vector<Result> client::Client::read_struct<Result>();
