#include "client.h"

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

tt::chat::client::Client::Client(int port, const std::string &server_address)
    : socket_{tt::chat::net::create_socket()} {
    sockaddr_in address = create_server_address(server_address, port);
    connect_to_server(socket_, address);
}

void tt::chat::client::Client::send_message(const std::string &message) {
    std::string len = std::to_string(message.length());
    while(len.size() < 20) len += '\0';

    ssize_t bytes_sent = send(socket_, len.c_str(), len.length(), 0);
    if (bytes_sent < 0) {
        tt::chat::check_error(true, "Send failed on client socket.");
    }
    bytes_sent = send(socket_, message.c_str(), message.length(), 0);
    if (bytes_sent < 0) {
        tt::chat::check_error(true, "Send failed on client socket.");
    }
}

int tt::chat::client::Client::get_socket_fd() const {
    return socket_;
}
tt::chat::client::Client::~Client() { close(socket_); }

sockaddr_in tt::chat::client::Client::create_server_address(
    const std::string &server_ip, int port) {
    using namespace tt::chat;
    sockaddr_in address = net::create_address(port);
    // Convert the server IP address to a binary format
    auto err_code = inet_pton(AF_INET, server_ip.c_str(), &address.sin_addr);
    check_error(err_code <= 0, "Invalid address/ Address not supported\n");
    return address;
}

void tt::chat::client::Client::connect_to_server(
    int sock, sockaddr_in &server_address) {
    using namespace tt::chat;
    auto err_code =
        connect(sock, (sockaddr *)&server_address, sizeof(server_address));
    check_error(err_code < 0, "Connection Failed.\n");
}