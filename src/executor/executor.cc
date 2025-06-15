#include "query_engine.h"
// #include "../../processed_data/preproc.h"
#include "../utils/query.h"
#include <cmath>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <ranges>

Executor::Executor() {
    data.open("data/processed/trades-example.bin", std::ios::in | std::ios::binary);
    if (!data.is_open()) {
        std::cerr << "[Executor] Error: could not open trade data file.\n";
        return;
    }
    trades_size = data.seekg(0, std::ios::end).tellg() / 31;
    data.seekg(0, std::ios::beg);
}

bool Executor::read_trade_data(uint64_t ind, TradeData& trade) {
    data.seekg(ind * sizeof(TradeData), std::ios::beg);
    if (data.read(reinterpret_cast<char*>(&trade.symbol_id), sizeof(trade.symbol_id)) &&
        data.read(reinterpret_cast<char*>(&trade.created_at), sizeof(trade.created_at)) &&
        data.read(reinterpret_cast<char*>(&trade.trade_id), sizeof(trade.trade_id)) &&
        data.read(reinterpret_cast<char*>(&trade.price.price), sizeof(trade.price.price)) &&
        data.read(reinterpret_cast<char*>(&trade.quantity.quantity), sizeof(trade.quantity.quantity)) &&
        data.read(reinterpret_cast<char*>(&trade.price.price_exponent), sizeof(trade.price.price_exponent)) &&
        data.read(reinterpret_cast<char*>(&trade.quantity.quantity_exponent), sizeof(trade.quantity.quantity_exponent)) &&
        data.read(reinterpret_cast<char*>(&trade.taker_side), sizeof(trade.taker_side))) {
        std::cout << trade.symbol_id << " " 
                  << trade.created_at << " "
                  << trade.trade_id << " "
                  << trade.price.price << " * 10^" 
                  << static_cast<int>(trade.price.price_exponent) << " "
                  << trade.quantity.quantity << " * 10^" 
                  << static_cast<int>(trade.quantity.quantity_exponent) << " "
                  << static_cast<int>(trade.taker_side) << "\n";
        return true;
    } else {
        throw std::runtime_error("[Executor] Error reading trade data at index " + std::to_string(ind));
    }
    return false; // If we reach here, it means reading failed
}

std::vector<Result> Executor::lowest_and_highest_prices(
    const TradeDataQuery& query) {

    std::vector<Result> result;

    // Early exit if no trades available
    // if (trades.empty()) {
    //     std::cout << "[Executor] No trades available.\n";
    //     return result;
    // }

    // Check if bit flag for min/max is enabled (bit 0)
    if ((query.metrics & (1 << 0)) == 0) {
        std::cout << "[Executor] Bit 0 (min/max) not set. Skipping computation.\n";
        return result;
    }

    assert(query.resolution > 0);

    // Calculate size of each time bucket based on resolution // Assuming resolution > 0
    uint64_t num_buckets = (query.end_time_point - query.start_time_point + (query.resolution - 1)) / query.resolution;


    // result = std::vector<Result>(num_buckets);

    double min_price_value = __DBL_MAX__, max_price_value = __DBL_MIN__;
    uint64_t bucket_start_time = query.start_time_point;
    uint64_t ind = 0;

    TradeData trade;

    for (uint64_t offset = query.start_time_point; offset < query.end_time_point; offset += query.resolution) {
        Price min_price = {0, 0};  // Reset min price
        Price max_price = {0, 0};  // Reset max price
        min_price_value = __DBL_MAX__;
        max_price_value = __DBL_MIN__;

        while (read_trade_data(ind, trade) && trade.created_at < query.start_time_point) ind++;

        while (trade.created_at < query.end_time_point && trade.created_at < offset + query.resolution) {
            double price_value = trade.price.price * std::pow(10, trade.price.price_exponent);

            if (price_value < min_price_value) {
                min_price_value = price_value;
                min_price = trade.price;
            } 
            if (price_value > max_price_value) {
                max_price_value = price_value;
                max_price = trade.price;
            }
            ind++;  // Move to the next trade
            if (ind >= trades_size) {
                break;  // No more trades to process
            }
        }

        result.push_back({
            bucket_start_time,
            min_price,
            max_price
        });
        bucket_start_time += query.resolution;
        ind++;  // Move to the next trade
        if (ind >= trades_size) {
            break;  // No more trades to process
        }
    }

    return result;
}


namespace ranges = std::ranges;

std::vector<TradeData> Executor::send_raw_data(TradeDataQuery &query)
{
    std::vector<TradeData> trades;

    for (uint64_t ind = 0; ind < trades_size; ind++) {
        TradeData trade;
        if (!read_trade_data(ind, trade)) {
            std::cerr << "[Executor] Error reading trade data at index " << ind << ".\n";
            continue;
        }
        if (trade.created_at >= query.start_time_point && trade.created_at < query.end_time_point) {
            trades.push_back(trade);
        }
    }
    return trades;
}


// int main(int argc, char** argv) {
//     if (argc < 2) {
//         std::cerr << "Usage: " << argv[0] << " <csv-file>\n";
//         return 1;
//     }
//     std::string filename = argv[1];
//     // auto trades = parse_csv(filename);

//     Executor executor;
//     std::cout << "Parsed " << trades.size() << " trades from " << filename << std::endl;
//     TradeDataQuery query = {
//         .symbol_id = 1,
//         .start_time_point = 1747267200000000000,
//         .end_time_point =   1747267200001855159, // 1 second in nanoseconds
//         .resolution = 1000000000000000000, // 100 milliseconds
//         .metrics = 1 // Enable min/max
//     };
//     auto results = executor.lowest_and_highest_prices(query);
//     std::cout << "Computed " << results.size() << " results for lowest and highest prices.\n";
//     for (const auto& res : results) {
//         std::cout << "Start Time: " << res.start_time << ", "
//                   << "Lowest Price: " << res.lowest_price.price << " * 10^" 
//                   << static_cast<int>(res.lowest_price.price_exponent) << ", "
//                   << "Highest Price: " << res.highest_price.price << " * 10^" 
//                   << static_cast<int>(res.highest_price.price_exponent) << "\n";
//     }
//     auto raw_data = executor.send_raw_data(query);
//     std::cout << "Retrieved " << raw_data.size() << " raw trades.\n";
//     for (const auto& trade : raw_data) {
//         std::cout << "Trade ID: " << trade.trade_id << ", "
//                   << "Symbol ID: " << trade.symbol_id << ", "
//                   << "Created At: " << trade.created_at << ", "
//                   << "Price: " << trade.price.price << " * 10^" 
//                   << static_cast<int>(trade.price.price_exponent) << ", "
//                   << "Quantity: " << trade.quantity.quantity << " * 10^" 
//                   << static_cast<int>(trade.quantity.quantity_exponent) << "\n";
//     }
//     std::cout << "Executor operations completed successfully.\n";
    
//     return 0;
// }

