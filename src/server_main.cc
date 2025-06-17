#include "server/server.h"
#include <spdlog/spdlog.h>  

int32_t main(int argc, char** argv) {
  if (argc < 2) {
    spdlog::error("Usage: {} <filename>", argv[0]);
    return 1;
  }
  int32_t kPort;
  kPort = 8080;
  int worker_threads = 1;
  std::string file = argv[1];
  EpollServer server(kPort, worker_threads, file);
  spdlog::info("Server started on port {}", kPort);
  server.run();
  return 0;
}
