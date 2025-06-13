#include "client.h"
#include "../utils/helper/utils.h"
#include "../utils/net/net.h"

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>


client::Client::Client(int port, const std::string &server_address)
    : socket_{net::create_socket()} {
    sockaddr_in address = create_server_address(server_address, port);
    connect_to_server(socket_, address);
}

void client::Client::send_message(const TradeDataQuery &message) {
    
    ssize_t bytes_sent = send(socket_, &message, sizeof(message), 0);
    if (bytes_sent < 0) {
        helper::check_error(true, "Send failed on client socket.");
    }
}

int client::Client::get_socket_fd() const {
    return socket_;
}
client::Client::~Client() { close(socket_); }

sockaddr_in client::Client::create_server_address(
    const std::string &server_ip, int port) {
    sockaddr_in address = net::create_address(port);
    // Convert the server IP address to a binary format
    auto err_code = inet_pton(AF_INET, server_ip.c_str(), &address.sin_addr);
    helper::check_error(err_code <= 0, "Invalid address/ Address not supported\n");
    return address;
}

void client::Client::connect_to_server(
    int sock, sockaddr_in &server_address) {
    auto err_code =
        connect(sock, (sockaddr *)&server_address, sizeof(server_address));
    helper::check_error(err_code < 0, "Connection Failed.\n");
}