#ifndef CHAT_SOCKETS_H
#define CHAT_SOCKETS_H

#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../helper/utils.h"

namespace tt::chat::net {

    int create_socket();

    sockaddr_in create_address(int port);

} // namespace tt::chat::net

#endif // CHAT_SOCKETS_H