#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <netinet/in.h>
#include <string>
#include "../utils/query.h"
#include <vector>

namespace client {
  class Client {
    // This class encapsulates the socket connection and basic communication
  public:
    /**
      * Constructs a Client and attempts to connect to the specified server.
      * @param port The port number of the server.
      * @param server_address The IP address or hostname of the server.
      * @throws std::runtime_error if socket creation or connection fails.
      */
    Client(uint16_t port, const std::string &server_address);
    /**
      * @brief Sends a message to the connected server.
      * @param message The message string to send.
      * @throws std::runtime_error if sending fails or client is not connected.
      */
    void send_message(const TradeDataQuery &message);
    /**
      * @brief Receive any struct from the server
      * 
      * @tparam T 
      * @return std::vector<T> 
      */
    template<typename T>
    std::vector<T> read_struct();
    /**
      * @brief Get the socket fd object
      * 
      * @return int32_t 
      */
    int32_t get_socket_fd() const; // Getter for the socket
    // Destroys the Client object, ensuring the socket is closed.
    ~Client();

  private:
    int32_t socket_;
    sockaddr_in server_address_;
    /**
      * Creates a server address structure (sockaddr_in).
      * @param server_ip The IP address of the server.
      * @param port The port number.
      * @return A configured sockaddr_in structure.
      * @throws std::runtime_error on invalid address.
      */
    sockaddr_in create_server_address(const std::string &server_ip, uint16_t port);
    /**
      * @brief Connects the client's socket to the specified server address.
      * @param server_address The sockaddr_in structure representing the server.
      * @throws std::runtime_error if connection fails.
      */
    void connect_to_server(int32_t sock, sockaddr_in &server_address);

    static constexpr int32_t kBufferSize = 1024;
  };
} // namespace client

#endif // CHAT_CLIENT_H
