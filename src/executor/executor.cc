#include "executor.h"
#include "../../processed_data/preproc.h"
#include "../utils/query.h"
#include <cmath>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <ranges>

uint64_t int_ceil(uint64_t x, uint64_t y){
    return (x/y) + (x%y != 0);
}

std::vector<Result> Executor::lowest_and_highest_prices(
    const TradeDataQuery& query) {
    std::vector<Result> result;

    // Early exit if no trades available
    if (trades.empty()) {
        std::cout << "[Executor] No trades available.\n";
        return result;
    }

    // Check if bit flag for min/max is enabled (bit 0)
    if ((query.metrics & (1 << 0)) == 0) {
        std::cout << "[Executor] Bit 0 (min/max) not set. Skipping computation.\n";
        return result;
    }

    assert(query.resolution > 0);

    // Calculate size of each time bucket based on resolution // Assuming resolution > 0
    uint64_t num_buckets = (query.end_time_point - query.start_time_point + (query.resolution - 1)) / query.resolution;
    
    std::cout << "Num buckets = " << num_buckets << std::endl;

    // result = std::vector<Result>(num_buckets);

    double min_price_value = __DBL_MAX__, max_price_value = __DBL_MIN__;
    uint64_t bucket_start_time = query.start_time_point;
    uint64_t ind = 0;

    for (uint64_t offset = query.start_time_point; offset < query.end_time_point; offset += query.resolution) {
        if(ind >= trades.size()) break;

        if(offset < trades[ind].created_at){
            uint64_t times = int_ceil(trades[ind].created_at-offset+1, query.resolution)-1;
            offset += query.resolution*times;
        }

        Price min_price = {0, 0};  // Reset min price
        Price max_price = {0, 0};  // Reset max price
        min_price_value = __DBL_MAX__;
        max_price_value = __DBL_MIN__;

        while (ind < trades.size() && trades[ind].created_at < query.start_time_point) ind++;

        while (ind < trades.size() && trades[ind].created_at < query.end_time_point && trades[ind].created_at < offset + query.resolution) {
            double price_value = trades[ind].price.price * std::pow(10, trades[ind].price.price_exponent);

            if (price_value < min_price_value) {
                min_price_value = price_value;
                min_price = trades[ind].price;
            } 
            if (price_value > max_price_value) {
                max_price_value = price_value;
                max_price = trades[ind].price;
            }
            ind++;  // Move to the next trade
            if (ind >= trades.size()) {
                break;  // No more trades to process
            }
        }

        std::cout << "Offset = " << offset << std::endl;
        std::cout << "Min price ";
        std::cout << min_price.price << std::endl;

        if(min_price.price != 0){
            result.push_back({
                bucket_start_time,
                min_price,
                max_price
            });
        }

        bucket_start_time += query.resolution;
    }

    std::cout << "Size of result = " << result.size() << std::endl;

    return result;
}


namespace ranges = std::ranges;

std::vector<TradeData> Executor::send_raw_data(TradeDataQuery &query)
{
  std::sort(trades.begin(), trades.end(), [](const TradeData& a, const TradeData& b) {
        return a.created_at < b.created_at;
    });
  auto it_start = std::ranges::lower_bound(trades, query.start_time_point, {}, &TradeData::created_at);
  auto it_end = std::ranges::lower_bound(trades, query.end_time_point, {}, &TradeData::created_at);

  std::cout << "[Executor] Returning trades between " 
            << query.start_time_point << " and " << query.end_time_point << ". "
            << "Found: " << std::distance(it_start, it_end) << " trades.\n";

  return std::vector<TradeData>(it_start, it_end);
}


// int main(int argc, char** argv) {
//     if (argc < 2) {
//         std::cerr << "Usage: " << argv[0] << " <csv-file>\n";
//         return 1;
//     }
//     std::string filename = argv[1];
//     auto trades = parse_csv(filename);

//     Executor executor(trades);
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

