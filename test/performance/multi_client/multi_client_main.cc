#include <iostream>     
#include <string>
#include <thread>
#include <atomic>
#include <csignal>      
#include <memory>      
#include <vector>       
#include <optional>

#include <spdlog/spdlog.h>
#include <unistd.h>     
#include <sys/socket.h> 
#include <errno.h>      

#include "multi_client.h"
#include "../../../src/utils/query.h"

#include <cmath> // for std::pow
std::atomic<bool> g_client_running{true};

bool one_shot = true;

int32_t main(int32_t argc, char* argv[]) {
  // Basic command line argument parsing
  std::string server_ip = "127.0.0.1";
  int32_t port = 8080;

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

//   spdlog::set_level(spdlog::level::info);
//   spdlog::info("Command-line Chat Client starting to connect to {}:{}", server_ip, port);

  std::optional<client::Client> chat_client;
  try {
      int32_t client_count = 5;
      chat_client.emplace(port, server_ip, client_count);
    //   std::cout << "Connected to server. Type messages or '/quit' to exit." << std::endl;
  } catch (const std::runtime_error& e) {
    //   spdlog::critical("Failed to create or connect client: {}", e.what());
    //   std::cerr << "Error connecting to server: " << e.what() << std::endl;
      return EXIT_FAILURE;
  } catch (...) {
    //   spdlog::critical("An unknown error occurred during client initialization.");
    //   std::cerr << "An unknown error occurred during client initialization." << std::endl;
      return EXIT_FAILURE;
  }

  while (g_client_running) {
    TradeDataQuery query;
    std::cin >> query.symbol_id >> query.start_time_point >> query.end_time_point >> query.resolution >> query.metrics;

    chat_client->send_message(query);
    std::vector<std::vector<Result>> output = chat_client->read_min_max();

    for (size_t i = 0; i < output.size(); ++i) {
        std::cout << "Socket " << i << " output:\n";
        for (const Result &data : output[i]) {
            int low_exp = static_cast<int>(data.lowest_price.price_exponent);
            int high_exp = static_cast<int>(data.highest_price.price_exponent);

            std::cout << "  Timestamp: " << data.start_time
                      << "; Min: " << data.lowest_price.price << "e" << low_exp
                      << "; Max: " << data.highest_price.price << "e" << high_exp
                      << "\n";
        }
    }
    // if (one_shot == true) break;
  }

  return EXIT_SUCCESS;
}
