#include "net.h"

int tt::chat::net::create_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    tt::chat::check_error(sock < 0, "Socket creation error\n");
    return sock;
}

sockaddr_in tt::chat::net::create_address(int port) {
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    return address;
}