#include "net.h"

int32_t net::create_socket() {
    int32_t sock = socket(AF_INET, SOCK_STREAM, 0);
    helper::check_error(sock < 0, "Socket creation error\n");
    return sock;
}

sockaddr_in net::create_address(int32_t port) {
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    return address;
}
