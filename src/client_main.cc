#include <iostream>     
#include <string>
#include <thread>
#include <atomic>
#include <csignal>      
#include <memory>      
#include <vector>       
#include <optional>
#include <sstream>

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
    
    std::vector<char> output = chat_client->read_struct<char>();
    std::ostringstream oss;

    if (output.empty()) {
        oss << std::endl;  // Always at least one line
    } else {
        size_t set_size = 0; // Each result set is 20 bytes (10 + 5 + 5)

        int8_t metric_list = 0;
        if (query.metrics & (1 << 0)) {
            metric_list |= (1 << 0); // min and max price
            set_size += 10;
        }
        if (query.metrics & (1 << 26)) {
            metric_list |= (1 << 1); // mean price
            set_size += 5;
        }
        if (query.metrics & (1ull << 33)) {
            metric_list |= (1 << 2); // total quantity
            set_size += 5;
        }

        size_t num_sets = output.size() / set_size;


        

        for (size_t set = 0; set < num_sets; ++set) {
            size_t base = set * set_size;
            int32_t index = 0;
            oss << "\nSet " << set + 1 << ":";
            for (int i=0 ; i<8 ; i++){
              if (metric_list & (1 << i)) {
                switch (i) {
                  case 0: { // min and max price
                    uint32_t min_price = *reinterpret_cast<const uint32_t*>(&output[base + index]);
                    int8_t min_exp = output[base + index + 4];
                    index += 5;
                    uint32_t max_price = *reinterpret_cast<const uint32_t*>(&output[base + index]);
                    int8_t max_exp = output[base + index + 4];
                    index += 5;
                    oss << "\n  Min Price: " << min_price << "e" << static_cast<int32_t>(min_exp)
                        << ", Max Price: " << max_price << "e" << static_cast<int32_t>(max_exp);
                    break;
                  }
                  case 1: { // mean price
                    uint32_t mean_price = *reinterpret_cast<const uint32_t*>(&output[base + index]);
                    int8_t mean_exp = output[base + index + 4];
                    index += 5;
                    oss << "\n  Mean Price: " << mean_price << "e" << static_cast<int32_t>(mean_exp);
                    break;
                  }
                  case 2: { // total quantity
                    uint32_t total_quantity = *reinterpret_cast<const uint32_t*>(&output[base + index]);
                    int8_t total_exp = output[base + index + 4];
                    index += 5;
                    oss << "\n  Total Quantity: " << total_quantity << "e" << static_cast<int32_t>(total_exp);
                    break;
                  }
                  default:
                    oss << "\n  Metric " << i+1 << ": Not implemented";
                }
              }
            }
            oss << "\n";  // Newline after each set
            // uint32_t min_price = *reinterpret_cast<const uint32_t*>(&output[base + 0]);
            // int8_t min_exp = output[base + 4];
            // uint32_t max_price = *reinterpret_cast<const uint32_t*>(&output[base + 5]);
            // int8_t max_exp = output[base + 9];
            // uint32_t mean_price = *reinterpret_cast<const uint32_t*>(&output[base + 10]);
            // int8_t mean_exp = output[base + 14];
            // uint32_t total_quantity = *reinterpret_cast<const uint32_t*>(&output[base + 15]);
            // int8_t total_exp = output[base + 19];

            // oss << "\nSet " << set + 1 << ":";
            // oss << "\n  Min Price: " << min_price << "e" << static_cast<int32_t>(min_exp)
            //     << ", Max Price: " << max_price << "e" << static_cast<int32_t>(max_exp);
            // oss << "\n  Mean Price: " << mean_price << "e" << static_cast<int32_t>(mean_exp);
            // oss << "\n  Total Quantity: " << total_quantity << "e" << static_cast<int32_t>(total_exp);
        }
      }

    std::cout << oss.str();  // Dump everything at once
    std::cout.flush();       // Ensure immediate flush
    #ifdef TESTMODE
      break;
    #endif
  }

  return EXIT_SUCCESS;
}