#include <iostream>     
#include <string>
#include <thread>
#include <atomic>
#include <csignal>      
#include <memory>      
#include <vector>       

#include <spdlog/spdlog.h>
#include <unistd.h>     
#include <sys/socket.h> 
#include <errno.h>      

#include "client/client.h"
#include "utils/query.h"

std::atomic<bool> g_client_running{true};

int main(int argc, char* argv[]) {
  // Basic command line argument parsing
  std::string server_ip = "127.0.0.1";
  int port = 8080;

  if (argc > 1) {
      server_ip = argv[1];
  }
  if (argc > 2) {
      try {
          port = std::stoi(argv[2]);
      } catch (const std::exception& e) {
          std::cerr << "Invalid port number: " << argv[2] << ". Using default " << port << std::endl;
      }
  }

  spdlog::set_level(spdlog::level::info);
  spdlog::info("Command-line Chat Client starting to connect to {}:{}", server_ip, port);

  std::optional<client::Client> chat_client;
  try {
      chat_client.emplace(port, server_ip);
      std::cout << "Connected to server. Type messages or '/quit' to exit." << std::endl;
  } catch (const std::runtime_error& e) {
      spdlog::critical("Failed to create or connect client: {}", e.what());
      std::cerr << "Error connecting to server: " << e.what() << std::endl;
      return EXIT_FAILURE;
  } catch (...) {
      spdlog::critical("An unknown error occurred during client initialization.");
      std::cerr << "An unknown error occurred during client initialization." << std::endl;
      return EXIT_FAILURE;
  }

  int client_socket_fd = chat_client->get_socket_fd();
  // std::thread reader_thread(read_loop, client_socket_fd);

  while (g_client_running) {
    TradeDataQuery query;
    std::cin >> query.symbol_id >> query.start_time_point >> query.end_time_point >> query.resolution >> query.metrics;

    chat_client->send_message(query);
    std::vector<Result> output = chat_client->read_min_max();

    for(Result data : output) {
      std::cout << "Timestamp: " << data.start_time 
        << "; Min Price: " << data.lowest_price.price << "e" << data.lowest_price.price_exponent
        << "; Max Price: " << data.highest_price.price << "e" << data.highest_price.price_exponent
        << std::endl;
    }
  }

  return EXIT_SUCCESS;
}