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


class CircularBuffer {
private:
    char buffer[1024];
    size_t head_;
    size_t tail_;
    size_t size_;
    static constexpr size_t capacity_ = sizeof(buffer);
public:
    CircularBuffer() : head_(0), tail_(0) {}

    void push(const char* data, size_t size) {
        if (size > 1024-size_) {
            throw std::runtime_error("Data size exceeds buffer capacity");
        }
        for (size_t i = 0; i < size; ++i) {
            buffer[head_] = data[i];
            head_ = (head_ + 1) % capacity_;
            if (head_ == tail_) { 
                tail_ = (tail_ + 1) % capacity_;
            }
        }
        size_+=size;
    }
    template<typename T>
    T read() {
      if (size_ < sizeof(T)) {
        throw std::runtime_error("Not enough data in buffer to read");
      }
      if (tail_ + sizeof(T) > capacity_) {
        char buf[sizeof(T)];
        for (size_t i = 0; i < sizeof(T); ++i) {
          buf[i] = buffer[(tail_ + i) % capacity_];
        }
        tail_ = (tail_ + sizeof(T)) % capacity_;
        size_ -= sizeof(T);
        return *reinterpret_cast<T*>(buf);
      }
      T value = *reinterpret_cast<T*>(buffer + tail_);
      tail_ = (tail_ + sizeof(T)) % capacity_;
      size_-=sizeof(T);
      return value;
    }
    template<typename T>
    T read_at() {
        if (tail_+sizeof(T)>= capacity_) {
          char buf[sizeof(T)];
          for (size_t i = 0; i < sizeof(T); ++i) {
            buf[i] = buffer[(tail_+ i) % capacity_];
          }
          return *reinterpret_cast<T*>(buf);
        }
        T value = *reinterpret_cast<T*>(buffer + tail_);
        return value;
    }
    
    int get_current_capacity() const {
        return capacity_;
    }

    char* get_head() {
        return buffer + head_;
    }

    int get_size() const {
        return size_;
    }

    int get_remaining_size() const {
        return capacity_ - size_;
    }

    void reduce_size(int size) {
        if (size > this->size_) {
            throw std::runtime_error("Cannot reduce size beyond current size");
        }
        tail_ = (tail_ + size) % capacity_;
        size_ -= size;
    }
};


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
  char SIZE[64] = {0}; // Size of each metric in bytes
  SIZE[0] = 2*sizeof(Price); // Lowest and highest prices
	SIZE[26] = sizeof(Price); // Mean price
	SIZE[33] = sizeof(Quantity); // Total quantity

  if (fcntl(chat_client->get_socket_fd(), F_SETFL, O_NONBLOCK) < 0) {
    spdlog::error("Failed to set socket to non-blocking mode: {}", strerror(errno));
    return EXIT_FAILURE;
  }

  CircularBuffer buffer;
  char buffer_data[1024];;
  int size_read;
  while (g_client_running) {
    TradeDataQuery query;
    std::cin >> query.symbol_id >> query.start_time_point >> query.end_time_point >> query.resolution >> query.metrics;
    std::cout << "Query: Symbol ID: " << query.symbol_id
              << ", Start Time: " << query.start_time_point
              << ", End Time: " << query.end_time_point
              << ", Resolution: " << query.resolution
              << ", Metrics: " << query.metrics << std::endl;
    chat_client->send_message(query);
    uint32_t size_of_each_result = sizeof(uint64_t); // Start time
    for (int8_t i = 0; i < 64; i++) {
      size_of_each_result += SIZE[i];
    }
    // bool taking_data = true;
    while(true) {
      size_read = recv(chat_client->get_socket_fd(), buffer_data, buffer.get_remaining_size(), 0);
      // std::cout << "size_read = " << size_read << std::endl;
      //assume terminal char is always present
      if (size_read < 0) {
        continue;
      } else if (size_read == 0) {
        // Server closed the connection
        spdlog::info("Server closed the connection.");
        return EXIT_SUCCESS;
      } else{
        std::cout << "Received " << size_read << " bytes from server." << std::endl;
        buffer.push(buffer_data, size_read);
        uint64_t start_time = buffer.read_at<uint64_t>();
        std::cout << "Received data, start time: " << start_time << std::endl;
        if (start_time == 0) {
          std::cout << "Breaking out of loop, no more data." << std::endl;
          break;
        }
        while (buffer.get_size() >= size_of_each_result) {
          uint64_t start_time = buffer.read<uint64_t>();
          std::cout << "Start Time: " << start_time << std::endl;
          int offset = sizeof(uint64_t); // Start time
          if (query.metrics & (1 << 0)) {
            Price min_price = buffer.read<Price>();
            offset += sizeof(Price);
            Price max_price = buffer.read<Price>();
            offset += sizeof(Price);
              std::cout << "Min Price: " << min_price .price
                << "e" << static_cast<int32_t>(min_price.price_exponent)
                << " Max Price: " << max_price.price
                << "e" << static_cast<int32_t>(max_price.price_exponent) << std::endl;
          }
          if (query.metrics & (1 << 26)) {
            Price mean_price = buffer.read<Price>();
            offset += sizeof(Price);
            std::cout << "Mean Price: " << mean_price.price
                      << "e" << static_cast<int32_t>(mean_price.price_exponent) << std::endl;
          }
          if (query.metrics & (1ULL << 33)) {
            Quantity total_quantity = buffer.read<Quantity>();
            offset += sizeof(Quantity);
            std::cout << "Total Quantity: " << total_quantity.quantity
                      << "e" << static_cast<int32_t>(total_quantity.quantity_exponent) << std::endl;
          }
        }
        start_time = buffer.read_at<uint64_t>();
        // std::cout << "Received data, start time: " << start_time << std::endl;
        if (start_time == 0) {
          std::cout << "Breaking out of loop, no more data." << std::endl;
          break;
        }
      }
    }
    #ifdef TESTMODE
      break;
    #endif
  }

  return EXIT_SUCCESS;
}
