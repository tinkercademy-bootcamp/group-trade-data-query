#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <netinet/in.h>
#include <string>
#include "../../../src/utils/query.h"
#include <vector>
#include <sys/epoll.h>
#include <fcntl.h>

namespace client {
    class Client {
    public:
        Client(int32_t port, const std::string &server_address, int32_t client_count = 1);
        void send_message(const TradeDataQuery &message);
        std::vector<std::vector<Result>> read_min_max();
        const std::vector<int32_t>& get_socket_fds() const;
        ~Client();

    private:
        std::vector<int32_t> sockets;
        int epoll_fd_;

        sockaddr_in create_server_address(const std::string &server_ip, int32_t port);
        void connect_to_server(int32_t sock, sockaddr_in &server_address);
        void set_non_blocking(int32_t fd);

        static constexpr int32_t kBufferSize = 1024;
    };
}


#endif
