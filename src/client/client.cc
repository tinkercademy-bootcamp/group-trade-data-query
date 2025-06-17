/**
 * @file client.cc
 * @brief Implements the client::Client class for TCP communication with the server.
 */

#include "client.h"
#include "../utils/helper/utils.h"
#include "../utils/net/net.h"

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

/**
 * @brief Constructs a Client and connects to the specified server.
 * @param port The server port.
 * @param server_address The server IP address.
 * @throws std::runtime_error if socket creation or connection fails.
 */
client::Client::Client(uint16_t port, const std::string &server_address)
  : socket_{net::create_socket()} {
  sockaddr_in address = create_server_address(server_address, port);
  connect_to_server(socket_, address);
}

/**
 * @brief Sends a TradeDataQuery message to the server.
 * @param message The TradeDataQuery struct to send.
 * @throws std::runtime_error if sending fails.
 */
void client::Client::send_message(const TradeDataQuery &message) {
  ssize_t bytes_sent = send(socket_, &message, sizeof(message), 0);
  if (bytes_sent < 0) {
    helper::check_error(true, "Send failed on client socket.");
  }
}

/**
 * @brief Gets the underlying socket file descriptor.
 * @return The socket file descriptor.
 */
int32_t client::Client::get_socket_fd() const {
  return socket_;
}

/**
 * @brief Destructor for Client. Closes the socket if open.
 */
client::Client::~Client() { close(socket_); }

/**
 * @brief Creates a sockaddr_in structure for the server address.
 * @param server_ip The server IP address as a string.
 * @param port The server port.
 * @return sockaddr_in structure for the server.
 * @throws std::runtime_error if address conversion fails.
 */
sockaddr_in client::Client::create_server_address(
  const std::string &server_ip, uint16_t port) {
  sockaddr_in address = net::create_address(port);
  // Convert the server IP address to a binary format
  auto err_code = inet_pton(AF_INET, server_ip.c_str(), &address.sin_addr);
  helper::check_error(err_code <= 0, "Invalid address/ Address not supported\n");
  return address;
}

/**
 * @brief Connects the client's socket to the specified server address.
 * @param sock The socket file descriptor.
 * @param server_address The sockaddr_in structure for the server.
 * @throws std::runtime_error if connection fails.
 */
void client::Client::connect_to_server(
  int32_t sock, sockaddr_in &server_address) {
  int32_t err_code = connect(sock, (sockaddr *)&server_address, sizeof(server_address));
  helper::check_error(err_code < 0, "Connection Failed.\n");
}

/**
 * @brief Reads a vector of structs of type T from the server.
 *        The server first sends an int32_t count, followed by count elements of type T.
 * @tparam T The struct type to read.
 * @return std::vector<T> The received data.
 * @throws std::runtime_error if reading fails.
 */
template<typename T>
std::vector<T> client::Client::read_struct() {
  int32_t count;
  ssize_t n = recv(socket_, &count, sizeof(count), 0);

  helper::check_error(n < 0, "Failed reading the size.\n");

  std::vector<T> output(count);
  for(int32_t i=0; i<count; i++) {
    n = recv(socket_, &(output[i]), sizeof(output[i]), 0);
    helper::check_error(n < 0, "Failed reading a Result struct.\n");
  }

  return output;
}

// Explicit template instantiations for supported types
template std::vector<TradeData> client::Client::read_struct<TradeData>();
template std::vector<Result>    client::Client::read_struct<Result>();
template std::vector<char>      client::Client::read_struct<char>();

