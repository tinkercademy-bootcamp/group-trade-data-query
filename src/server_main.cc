#include "server/server.h"

int main(){
  int kPort;
  kPort = 8080;
  EpollServer server(kPort);
  spdlog::info("Server started on port {}", kPort);
  server.run();
  return 0;
}