#ifndef NET_H
#define NET_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "../helper/utils.h"

namespace net {

inline int32_t create_socket() {
  int32_t sock = socket(AF_INET, SOCK_STREAM, 0);
  helper::check_error(sock < 0, "Socket creation error\n");
  return sock;
}

inline sockaddr_in create_address(uint16_t port) {
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  return address;
}

}  // namespace net

#endif
