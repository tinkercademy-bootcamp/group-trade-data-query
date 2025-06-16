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

int main() {
    Query_engine executor;
    std::ofstream out("data/processed/page-table-example.bin", std::ios::binary);

    if (!out.is_open()) {
        std::cerr << "Error: could not open output file for writing." << std::endl;
        return 1;
    }

    int64_t ind = 0;
    TradeData trade;
    pte_t pte;
    while (ind < executor.trades_size) {
        executor.read_trade_data(ind, trade);
        pte.created_at = trade.created_at;
        pte.index = ind;
        out.write(reinterpret_cast<const char*>(&pte), sizeof(pte_t));
        ind += 128; // Assuming each page contains 128 trades
    }
    out.close();
    std::cout << "Page table created successfully with " << ind / 128 << " pages." << std::endl;
}