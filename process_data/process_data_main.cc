#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include "../src/utils/query.h"
#include "../src/data_structures/segment_tree.h"
#include "../src/query_engine/query_engine.h"

/**
 * @brief Parse a CSV file and return a vector of TradeData structs.
 * @param filename Path to the CSV file
 * @return Vector of parsed TradeData records
 */
void parse_csv(const std::string& filename, std::ofstream& out) {
  std::vector<TradeData> trades;
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: could not open file " << filename << std::endl;
    return;
  }

  std::string line;
  // Skip header
  if (!std::getline(file, line)) {
    return;
  }
  TradeData trade;
  while (std::getline(file, line)) {
    if (line.empty() || line == "symbol_id,created_at,trade_id,price,quantity,price_exponent,quantity_exponent,taker_side\n") {
      continue; // Skip empty lines or header
    }

    std::istringstream ss(line);
    std::string token;

    std::getline(ss, token, ',');
    uint32_t symbol_id = static_cast<uint32_t>(std::stoul(line.substr(0, line.find(','))));

    std::getline(ss, token, ',');
    uint64_t created_at = static_cast<uint64_t>(std::stoull(token));

    std::getline(ss, token, ',');
    uint64_t trade_id = static_cast<uint64_t>(std::stoull(token));

    std::getline(ss, token, ',');
    uint32_t price_raw = static_cast<uint32_t>(std::stoul(token));
        
        
    std::getline(ss, token, ',');
    uint32_t quantity_raw = static_cast<uint32_t>(std::stoul(token));
    std::getline(ss, token, ',');
    uint8_t price_exponent = static_cast<int8_t>(std::stoi(token));

    std::getline(ss, token, ',');
    uint8_t quantity_exponent = static_cast<int8_t>(std::stoi(token));

    std::getline(ss, token, ',');
    uint8_t taker_side = 0;
    if (token == "Ask" || token == "ask" || token == "1") {
      taker_side = 1;
    } else if (token == "Bid" || token == "bid" || token == "2") {
      taker_side = 2;
    } else {
      taker_side = static_cast<uint8_t>(std::stoul(token));
    }

    // std::cout << sizeof(TradeData) << std::endl;

    trade.symbol_id = symbol_id;
    trade.created_at = created_at;
    trade.trade_id = trade_id;
    trade.price.price = price_raw;
    trade.price.price_exponent = price_exponent;
    trade.quantity.quantity = quantity_raw;
    trade.quantity.quantity_exponent = quantity_exponent;
    trade.taker_side = taker_side;

    out.write(reinterpret_cast<const char *>(&trade), sizeof(TradeData));
  }
}
// Simple test to verify parsing
int32_t main(int32_t argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <csv-file>\n";
    return 1;
  }
  std::string filename = argv[1];

  std::string dir = filename.substr(0, filename.find_last_of('/'));
  std::string base_name = filename.substr(filename.find_last_of('/') + 1);
  std::string parent_dir = dir.substr(0, dir.find_last_of('/'));

  {
    std::string out_path = parent_dir + "/processed/" + base_name.substr(0, base_name.find_last_of('.')) + ".bin";

    std::ofstream out(out_path, std::ios::out | std::ios::trunc | std::ios::binary);

    if (!out.is_open()) {
      std::cerr << "Error: could not open output file\n";
      return 1;
    }

    parse_csv(filename, out);
  }
  
  return 0;
}
