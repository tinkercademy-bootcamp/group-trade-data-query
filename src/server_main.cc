#include "server/server.h"
#include <spdlog/spdlog.h>  

int32_t main(){
  int32_t kPort;
  kPort = 8080;
  EpollServer server(kPort, 31);
  spdlog::info("Server started on port {}", kPort);
  server.run();
  return 0;
}
