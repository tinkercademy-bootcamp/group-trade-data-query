#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../src/utils/query.h"

/**
 * @brief Parse a CSV file and return a vector of TradeData structs.
 * @param filename Path to the CSV file
 * @return Vector of parsed TradeData records
 */
std::vector<TradeData> parse_csv(const std::string& filename) {
    std::vector<TradeData> trades;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << filename << std::endl;
        return trades;
    }

    std::string line;
    // Skip header
    if (!std::getline(file, line))
        return trades;

    // Read lines
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string token;
        TradeData t;

        // symbol_id
        std::getline(ss, token, ',');
        t.symbol_id = static_cast<uint32_t>(std::stoul(token));

        // created_at
        std::getline(ss, token, ',');
        t.created_at = static_cast<uint64_t>(std::stoull(token));

        // trade_id
        std::getline(ss, token, ',');
        t.trade_id = static_cast<uint64_t>(std::stoull(token));

        // price (raw)
        std::getline(ss, token, ',');
        t.price.price = static_cast<uint32_t>(std::stoul(token));

        // quantity (raw)
        std::getline(ss, token, ',');
        t.quantity.quantity = static_cast<uint32_t>(std::stoul(token));

        // price_exponent
        std::getline(ss, token, ',');
        t.price.price_exponent = static_cast<int8_t>(std::stoi(token));

        // quantity_exponent
        std::getline(ss, token, ',');
        t.quantity.quantity_exponent = static_cast<int8_t>(std::stoi(token));

        // taker_side
        std::getline(ss, token, ',');
        if (token == "Ask" || token == "ask" || token == "1")        t.taker_side = 1;
        else if (token == "Bid" || token == "bid" || token == "2")   t.taker_side = 2;
        else                                                             t.taker_side = static_cast<uint8_t>(std::stoul(token));

        trades.push_back(t);
    }

    return trades;
}

// Simple test to verify parsing
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <csv-file>\n";
        return 1;
    }
    std::string filename = argv[1];
    auto trades = parse_csv(filename);

    std::cout << "Parsed " << trades.size() << " trades from " << filename << std::endl;
    if (!trades.empty()) {
        const auto& t = trades[0];
        std::cout << "First record:\n"
                  << "  symbol_id       = " << t.symbol_id << "\n"
                  << "  created_at      = " << t.created_at << "\n"
                  << "  trade_id        = " << t.trade_id << "\n"
                  << "  price (raw)     = " << t.price.price << " * 10^" << int(t.price.price_exponent) << "\n"
                  << "  quantity (raw)  = " << t.quantity.quantity << " * 10^" << int(t.quantity.quantity_exponent) << "\n"
                  << "  taker_side      = " << int(t.taker_side) << "\n";
    }
    return 0;
}
