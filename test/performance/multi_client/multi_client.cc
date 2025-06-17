#include "multi_client.h"
#include "../../../src/utils/net/net.h"

client::Client::Client(int32_t port, const std::string &server_address, int32_t client_count) {
    epoll_fd_ = epoll_create1(0);
    helper::check_error(epoll_fd_ < 0, "Failed to create epoll instance");

    for (int i = 0; i < client_count; ++i) {
        int32_t sock = net::create_socket();
        sockaddr_in address = create_server_address(server_address, port);
        connect_to_server(sock, address);

        set_non_blocking(sock);
        sockets_.push_back(sock);

        epoll_event ev;
        ev.data.fd = sock;
        ev.events = EPOLLIN;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sock, &ev);
    }
}


void client::Client::send_message(const TradeDataQuery &message) {
    for (auto socket_ : sockets_){
      ssize_t bytes_sent = send(socket_, &message, sizeof(message), 0);
      if (bytes_sent < 0) {
          helper::check_error(true, "Send failed on client socket.");
      }
    }
}

const std::vector<int32_t>& client::Client::get_socket_fds() const {
    return sockets_;
}

client::Client::~Client() {
    for (auto socket : sockets_) {
        close(socket);
    }
    close(epoll_fd_);
}

sockaddr_in client::Client::create_server_address(
    const std::string &server_ip, int32_t port) {
    sockaddr_in address = net::create_address(port);
    // Convert the server IP address to a binary format
    auto err_code = inet_pton(AF_INET, server_ip.c_str(), &address.sin_addr);
    helper::check_error(err_code <= 0, "Invalid address/ Address not supported\n");
    return address;
}

void client::Client::connect_to_server(
    int32_t sock, sockaddr_in &server_address) {
    int32_t err_code =connect(sock, (sockaddr *)&server_address, sizeof(server_address));
    helper::check_error(err_code < 0, "Connection Failed.\n");
}

// the only test for now
std::vector<std::vector<Result>> client::Client::read_min_max() {
    std::vector<std::vector<Result>> result(sockets_.size());
    constexpr int MAX_EVENTS = 10;
    epoll_event events[MAX_EVENTS];

    int nfds = epoll_wait(epoll_fd_, events, MAX_EVENTS, 50000);
    helper::check_error(nfds < 0, "epoll_wait failed");

    for (int i = 0; i < nfds; ++i) {
        int fd = events[i].data.fd;

        int count;
        ssize_t n = recv(fd, &count, sizeof(count), 0);
        helper::check_error(n <= 0, "Failed to read count from socket");

        std::vector<Result> output(count);
        for (int j = 0; j < count; ++j) {
            n = recv(fd, &output[j], sizeof(Result), 0);
            helper::check_error(n <= 0, "Failed to read result");
        }

        // Find index of this socket in `sockets_`
        auto it = std::find(sockets_.begin(), sockets_.end(), fd);
        if (it != sockets_.end()) {
            result[it - sockets_.begin()] = std::move(output);
        }
    }

    return result;
}


void client::Client::set_non_blocking(int32_t fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
