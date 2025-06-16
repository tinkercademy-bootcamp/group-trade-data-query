// #include "server/server.h"
#include <spdlog/spdlog.h>

#include "server_udp/server_udp.h"

int32_t main() {
  int32_t kPort;
  kPort = 8080;
  EpollServer server(kPort);
  spdlog::info("Server started on port {}", kPort);
  server.run();
  return 0;
}
