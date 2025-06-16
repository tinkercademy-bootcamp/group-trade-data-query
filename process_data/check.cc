#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include "../src/utils/query.h"

int main() {
    std::ifstream file("data/processed/trades-example.bin");

    if (!file.is_open()) {
        std::cerr << "Error: could not open file trades-example.bin" << std::endl;
        return 1;
    }

    file.seekg(0, std::ios::beg);
    std::cout << "Current position: " << file.tellg() << std::endl;
    // file.seekg(10*sizeof(TradeData), std::ios::beg);

    TradeData trade;
    std::cout << file.tellg() << std::endl;
    file.read(reinterpret_cast<char*>(&trade), sizeof(TradeData));
    std::cout << "Trade Data:" << std::endl;
    std::cout << "Symbol ID: " << trade.symbol_id << std::endl;
    std::cout << "Created At: " << trade.created_at << std::endl;
    std::cout << "Trade ID: " << trade.trade_id << std::endl;
    std::cout << "Price: " << trade.price.price << " * 10^" 
              << static_cast<int>(trade.price.price_exponent) << std::endl;
    std::cout << "Quantity: " << trade.quantity.quantity << " * 10^"
                << static_cast<int>(trade.quantity.quantity_exponent) << std::endl;
    std::cout << "Taker Side: " << static_cast<int>(trade.taker_side) << std::endl;

    
    file.close();
    return 0;
}