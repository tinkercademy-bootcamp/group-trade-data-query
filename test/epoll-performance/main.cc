#include "antiserver.h"
#include <iostream>

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <host> <port> [clients]\n";
    return 1;
  }
  std::string host = argv[1];
  uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));
  int ncli = (argc > 3) ? std::stoi(argv[3]) : 256;

  try {
    AntiServer anti(host, port, ncli);
    anti.run(); // blocks forever
  } catch (const std::exception &e) {
    std::cerr << "Fatal: " << e.what() << '\n';
  }
  return 0;
}
