#ifndef NET_H
#define NET_H

#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../helper/utils.h"

namespace net {

    // Creates a socket and returns the socket file descriptor
    int create_socket();

    // Inputs port, creates address and returns it
    sockaddr_in create_address(int port);

} // namespace net

#endif

