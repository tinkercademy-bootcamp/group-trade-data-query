#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include "../src/utils/query.h"
#include "../src/query_engine/query_engine.h"

std::ifstream data;

bool read_trade_data(uint64_t ind, TradeData& trade) {
  data.seekg(ind * sizeof(TradeData), std::ios::beg);
  data.read(reinterpret_cast<char *>(&trade), sizeof(TradeData));
  return true; // If we reach here, it means reading failed
}

int32_t main(int32_t argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file-name>" << std::endl;
        std::cerr << "Example file names: example, tiny" << std::endl;
        return 1;
    }
    std::string filename = argv[1];
    // Query_engine query_engine(filename);
    std::ofstream outer_page_table_bin("data/processed/page-table-" + filename + "-outer.bin", std::ios::binary);

    data.open("data/processed/trades-" + filename + ".bin", std::ios::in | std::ios::binary);
    if (!data.is_open()) {
        std::cerr << "[Query_engine] Error: could not open trade data file.\n";
        return 0;
    }
    
    if (!outer_page_table_bin.is_open()) {
        std::cerr << "Error: could not open output file for writing." << std::endl;
        return 1;
    }

    std::ofstream inner_page_table_bin("data/processed/page-table-" + filename + "-inner.bin", std::ios::binary);

    if (!inner_page_table_bin.is_open()) {
        std::cerr << "Error: could not open inner output file for writing." << std::endl;
        return 1;
    }

    uint64_t trades_size = data.seekg(0, std::ios::end).tellg() / sizeof(TradeData);

    int64_t ind = 0;
    TradeData trade;
    while (ind < trades_size) {
        read_trade_data(ind, trade);
        outer_page_table_bin.write(reinterpret_cast<const char*>(&trade.created_at), sizeof(uint64_t));
        if (__builtin_ctzll(ind) >= 26) {
            inner_page_table_bin.write(reinterpret_cast<const char*>(&trade.created_at), sizeof(uint64_t));
        }
        ind += (1 << 17);
    }
    outer_page_table_bin.close();
    inner_page_table_bin.close();

    std::cout << "Page table created successfully with " << ind / 128 << " pages." << std::endl;
}