#include "socket_helper.h"
#include "../helper/error_handling.h"
#include <netinet/in.h>

int net::create_socket() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  helper::check_error(sock < 0, "Socket creation error\n");
  return sock;
}

sockaddr_in net::create_server_address(int port) {
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  return address;
}