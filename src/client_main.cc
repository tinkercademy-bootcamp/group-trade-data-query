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

#include "client/client.h"
// #include "client-udp/client.h"
#include "utils/query.h"

#include <cmath> // for std::pow
std::atomic<bool> g_client_running{true};

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
  #ifdef TESTMODE
    spdlog::set_level(spdlog::level::off);
  #else
    spdlog::set_level(spdlog::level::info);
  #endif
  spdlog::info("Command-line Chat Client starting to connect to {}:{}", server_ip, port);

  std::optional<client::Client> chat_client;
  try {
      chat_client.emplace(port, server_ip);
      spdlog::info("Connected to server.");
  } catch (const std::runtime_error& e) {
      spdlog::critical("Failed to create or connect client: {}", e.what());
      std::cerr << "Error connecting to server: " << e.what() << std::endl;
      return EXIT_FAILURE;
  } catch (...) {
      spdlog::critical("An unknown error occurred during client initialization.");
      std::cerr << "An unknown error occurred during client initialization." << std::endl;
      return EXIT_FAILURE;
  }

  int32_t client_socket_fd = chat_client->get_socket_fd();
  // std::thread reader_thread(read_loop, client_socket_fd);

  while (g_client_running) {
    TradeDataQuery query;
    std::cin >> query.symbol_id >> query.start_time_point >> query.end_time_point >> query.resolution >> query.metrics;

    chat_client->send_message(query);
    

    if(query.resolution == 0) {
        std::vector<TradeData> output = chat_client->read_struct<TradeData>();
        for (const TradeData& data : output) {

            std::cout << data.symbol_id << " " << data.created_at << " " << data.trade_id
                << " " << data.price.price << "e" << data.price.price_exponent << " " 
                << data.quantity.quantity << "e" << data.quantity.quantity_exponent << " "
                << data.taker_side << std::endl;
        }
    }
    else {
        if(query.metrics & (1<<0)) {
            std::vector<Result> output = chat_client->read_struct<Result>();
            for (const Result& data : output) {
                int low_exp = static_cast<int>(data.lowest_price.price_exponent);
                int high_exp = static_cast<int>(data.highest_price.price_exponent);

                std::cout << "Timestamp: " << data.start_time
                        << "; Min Price: " << data.lowest_price.price << "e" << low_exp
                        << "; Max Price: " << data.highest_price.price << "e" << high_exp
                        << std::endl;


            }
        }
    }
    
  }

  return EXIT_SUCCESS;
}
