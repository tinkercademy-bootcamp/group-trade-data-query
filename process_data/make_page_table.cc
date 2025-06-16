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

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file-name>" << std::endl;
        std::cerr << "Example file names: example, tiny" << std::endl;
        return 1;
    }
    std::string file = argv[1];
    Query_engine query_engine(file);
    std::ofstream out("data/processed/page-table-" + file + "-outer.bin", std::ios::binary);
    
    if (!out.is_open()) {
        std::cerr << "Error: could not open output file for writing." << std::endl;
        return 1;
    }

    std::ofstream out2("data/processed/page-table-" + file + "-inner.bin", std::ios::binary);

    if (!out2.is_open()) {
        std::cerr << "Error: could not open inner output file for writing." << std::endl;
        return 1;
    }

    int64_t ind = 0;
    TradeData trade;
    std::cout << sizeof(TradeData) << " bytes per trade" << std::endl;
    while (ind < query_engine.trades_size) {
        query_engine.read_trade_data(ind, trade);
        out.write(reinterpret_cast<const char*>(&trade.created_at), sizeof(uint64_t));
        ind += (1 << 17);
        if (ind % (1 << 26) == 0) {
            out2.write(reinterpret_cast<const char*>(&trade), sizeof(TradeData));
        }
    }
    out.close();
    out2.close();
    std::cout << "Page table created successfully with " << ind / 128 << " pages." << std::endl;
}