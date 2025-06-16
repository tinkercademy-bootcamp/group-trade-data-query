#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include "../src/utils/query.h"
#include "../src/query_engine/query_engine.h"
#include "../src/query_engine/page_table.h"

int32_t main(int32_t argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file-name>" << std::endl;
        std::cerr << "Example file names: example, tiny" << std::endl;
        return 1;
    }
    std::string filename = argv[1];
    Query_engine query_engine(filename);
    std::ofstream outer_page_table_bin("data/processed/page-table-" + filename + "-outer.bin", std::ios::binary);
    
    if (!outer_page_table_bin.is_open()) {
        std::cerr << "Error: could not open output file for writing." << std::endl;
        return 1;
    }

    std::ofstream inner_page_table_bin("data/processed/page-table-" + filename + "-inner.bin", std::ios::binary);

    if (!inner_page_table_bin.is_open()) {
        std::cerr << "Error: could not open inner output file for writing." << std::endl;
        return 1;
    }

    int64_t ind = 0;
    TradeData trade;
    while (ind < query_engine.trades_size) {
        query_engine.read_trade_data(ind, trade);
        outer_page_table_bin.write(reinterpret_cast<const char*>(&trade.created_at), sizeof(uint64_t));
        ind += (1 << 17);
        if (ind % (1 << 26) == 0) {
            inner_page_table_bin.write(reinterpret_cast<const char*>(&trade), sizeof(TradeData));
        }
    }
    outer_page_table_bin.close();
    inner_page_table_bin.close();
    std::cout << "Page table created successfully with " << ind / 128 << " pages." << std::endl;
}