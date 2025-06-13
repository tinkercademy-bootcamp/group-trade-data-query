// receive TradeDataQuery struct from the client using epoll
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../utils/query.h"

void receive_data(int sockfd, TradeDataQuery& query) {
    ssize_t bytes_received = recv(sockfd, &query, sizeof(query), 0);
    if (bytes_received < 0) {
        throw std::runtime_error("Failed to receive data: " + std::string(strerror(errno)));
    } else if (bytes_received < static_cast<ssize_t>(sizeof(query))) {
        std::cerr << "Warning: Only " << bytes_received << " out of " << sizeof(query) << " bytes received." << std::endl;
    }
}