#include "server/server.h"

int main(){
  EpollServer server(8080);
  server.run();
  return 0;
}