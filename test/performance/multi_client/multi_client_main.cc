#include <atomic>
#include <iostream>
#include <optional>
#include <string>

#include "../../nlohmann/json.hpp"
#include <algorithm>
#include <fstream>
#include <spdlog/spdlog.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

#include "../../../src/utils/query.h"
#include "multi_client.h"

std::atomic<bool> g_client_running{true};

bool one_shot = true;

int32_t main(int32_t argc, char *argv[]) {
  // Basic command line argument parsing
  std::string server_ip = "127.0.0.1";
  int32_t port = 8080;
  int32_t client_count = 10;

  if (argc > 1) {
    server_ip = argv[1];
  }
  if (argc > 2) {
    try {
      port = std::stoi(argv[2]);
    } catch (const std::exception &e) {
      std::cerr << "Invalid port number: " << argv[2] << ". Using default "
                << port << std::endl;
    }
  }
  if (argc > 3) {
    try {
      client_count = std::stoi(argv[3]);
    } catch (const std::exception &e) {
      std::cerr << "Invalid client count: " << argv[3] << ". Using default "
                << client_count << std::endl;
    }
  }

  spdlog::set_level(spdlog::level::info);
  spdlog::info(
      "Performance test client starting to connect to {}:{} with {} clients",
      server_ip, port, client_count);

  std::ifstream f("test/performance/performance_tests.json");
  if (!f.is_open()) {
    spdlog::error("Failed to open test/performance/performance_tests.json");
    return EXIT_FAILURE;
  }
  nlohmann::json data = nlohmann::json::parse(f);

  int passed_tests = 0;
  int total_tests = 0;
  
  for (const auto &test : data["tests"]) {
    total_tests++;
    std::string test_id = test["test_id"];
    std::string input_str = test["input"];
    std::string expected_output = test["expected_output"];

    spdlog::info("Running {}", test_id);

    std::optional<client::Client> chat_client;
    try {
      chat_client.emplace(port, server_ip, client_count);
    } catch (const std::runtime_error &e) {
      spdlog::error("Failed to create client for test {}: {}", test_id,
                    e.what());
      spdlog::error("{} FAILED", test_id);
      continue;
    }

    std::optional<TradeDataQuery> query = utils::string_to_query(input_str);
    if (!query) {
      spdlog::error("Failed to parse query from input: {}", input_str);
      spdlog::error("{} FAILED", test_id);
      continue;
    }

    chat_client->send_message(*query);

    std::vector<std::vector<Result>> output = chat_client->read_min_max();
    std::vector<Result> all_results;
    for (const auto &vec : output) {
      all_results.insert(all_results.end(), vec.begin(), vec.end());
    }
    std::sort(all_results.begin(), all_results.end(),
              [](const Result &a, const Result &b) {
                return a.start_time < b.start_time;
              });

    std::stringstream actual_output_ss;
    for (size_t i = 0; i < all_results.size(); ++i) {
      const auto &result_data = all_results[i];
      int low_exp = static_cast<int>(result_data.lowest_price.price_exponent);
      int high_exp = static_cast<int>(result_data.highest_price.price_exponent);
      actual_output_ss << "Timestamp: " << result_data.start_time
                       << "; Min Price: " << result_data.lowest_price.price
                       << "e" << low_exp
                       << "; Max Price: " << result_data.highest_price.price
                       << "e" << high_exp;
      if (i < all_results.size() - 1) {
        actual_output_ss << "\n";
      }
    }
    std::string actual_output = actual_output_ss.str();

    if (actual_output == expected_output) {
      spdlog::info("{} PASSED", test_id);
      passed_tests++;
    } else {
      spdlog::error("{} FAILED", test_id);
      spdlog::error("Expected output:\n{}", expected_output);
      spdlog::error("Actual output:\n{}", actual_output);
    }
  }

  spdlog::info("Test summary: {}/{} passed.", passed_tests, total_tests);

  return (passed_tests == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}