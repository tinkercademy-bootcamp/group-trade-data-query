#include "server/server.h"
#include <spdlog/spdlog.h>  

int32_t main(int argc, char** argv) {
  if (argc < 2) {
    spdlog::error("Usage: {} <filename>", argv[0]);
    return 1;
  }
  int32_t kPort;
  kPort = 8080;
  EpollServer server(kPort, argv[1]);
  spdlog::info("Server started on port {}", kPort);
  server.run();
  return 0;
}
