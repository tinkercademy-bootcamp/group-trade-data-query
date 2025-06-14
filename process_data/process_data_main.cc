#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include "../src/utils/query.h"

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
    if (!std::getline(file, line))
        return;

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
        uint8_t price_exponent = static_cast<int8_t>(std::stoi(token));

        std::getline(ss, token, ',');
        uint32_t quantity_raw = static_cast<uint32_t>(std::stoul(token));

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

        out.write(reinterpret_cast<const char *>(&symbol_id), sizeof(symbol_id));
        out.write(reinterpret_cast<const char *>(&created_at), sizeof(created_at));
        out.write(reinterpret_cast<const char *>(&trade_id), sizeof(trade_id));
        out.write(reinterpret_cast<const char *>(&price_raw), sizeof(price_raw));
        out.write(reinterpret_cast<const char *>(&price_exponent), sizeof(price_exponent));
        out.write(reinterpret_cast<const char *>(&quantity_raw), sizeof(quantity_raw));
        out.write(reinterpret_cast<const char *>(&quantity_exponent), sizeof(quantity_exponent));
        out.write(reinterpret_cast<const char *>(&taker_side), sizeof(taker_side));
        out.flush();
    }
}

// Simple test to verify parsing
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <csv-file>\n";
        return 1;
    }
    std::string filename = argv[1];

    std::string dir = filename.substr(0, filename.find_last_of('/'));
    std::string base_name = filename.substr(filename.find_last_of('/') + 1);
    std::string parent_dir = dir.substr(0, dir.find_last_of('/'));

    std::string out_path = parent_dir + "/processed/" + base_name.substr(0, base_name.find_last_of('.')) + ".bin";

    std::ofstream out(out_path, std::ios::out | std::ios::trunc | std::ios::binary);

    if (!out.is_open()) {
        std::cerr << "Error: could not open output file\n";
        return 1;
    }

    parse_csv(filename, out);

    return 0;
}
