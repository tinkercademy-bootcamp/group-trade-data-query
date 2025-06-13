#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>

void send_data(int sockfd, const std::string& data) {
    ssize_t bytes_sent = send(sockfd, data.c_str(), data.size(), 0);
    if (bytes_sent < 0) {
        throw std::runtime_error("Failed to send data: " + std::string(strerror(errno)));
    } else if (bytes_sent < static_cast<ssize_t>(data.size())) {
        std::cerr << "Warning: Only " << bytes_sent << " out of " << data.size() << " bytes sent." << std::endl;
    }
}