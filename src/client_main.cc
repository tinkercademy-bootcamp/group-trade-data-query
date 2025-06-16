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
    
    if (query.metrics & (1 << 0)) {
    std::vector<char> output = chat_client->read_struct<char>();
    std::ostringstream oss;

    if (output.empty()) {
        oss << std::endl;  // Always at least one line
    } else {
        oss << "Received " << output.size() << " items:\n";
        // for (const char& data : output) {
        //     oss << data << " ";
        // }
        // the result format is :
        // first 5 bytes are the min price, next 5 bytes are the max price
        // next 4 bytes are the mean price, next 4 bytes are the total quantity
        // Note: The above assumes the data is in a specific format, adjust as needed
        if (output.size() >= 10) {
            uint32_t min_price = *reinterpret_cast<const uint32_t*>(&output[0]);
            int8_t min_exp = output[4];
            uint32_t max_price = *reinterpret_cast<const uint32_t*>(&output[5]);
            int8_t max_exp = output[9];
            oss << "\nMin Price: " << min_price << "e" << static_cast<int32_t>(min_exp)
                << ", Max Price: " << max_price << "e" << static_cast<int32_t>(max_exp);
        }
        if (output.size() >= 14) {
            uint32_t mean_price = *reinterpret_cast<const uint32_t*>(&output[10]);
            int8_t mean_exp = output[14];
            oss << "\nMean Price: " << mean_price << "e" << static_cast<int32_t>(mean_exp);
        }
        if (output.size() >= 18) {
            uint32_t total_quantity = *reinterpret_cast<const uint32_t*>(&output[15]);
            int8_t total_exp = output[19];
            oss << "\nTotal Quantity: " << total_quantity << "e" << static_cast<int32_t>(total_exp);
        }
    }

    std::cout << oss.str();  // Dump everything at once
    std::cout.flush();       // Ensure immediate flush
}
    #ifdef TESTMODE
      break;
    #endif
  }

  return EXIT_SUCCESS;
}
