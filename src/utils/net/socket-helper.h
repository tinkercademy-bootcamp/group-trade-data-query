#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "../helper/error_handling.h"

namespace net {
int create_socket();
sockaddr_in create_server_address(int port);
}  // namespace net