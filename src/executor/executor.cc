#include "executor.h"

#include "../utils/query.h"
#include <cmath>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <ranges>

std::vector<std::pair<Price, Price>> Executor::lowest_and_highest_prices(
    const TradeDataQuery& query) {

    std::vector<std::pair<Price, Price>> result;

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


    result = std::vector<std::pair<Price, Price>>(num_buckets);

    double min_price_value = __DBL_MAX__, max_price_value = __DBL_MIN__;
    uint64_t bucket_start_time = query.start_time_point;
    uint64_t ind = 0;

    for (uint64_t offset = query.start_time_point; offset < query.end_time_point; offset += query.resolution) {
        Price min_price = {-1, 0};  // Reset min price
        Price max_price = {-1, 0};  // Reset max price
        min_price_value = __DBL_MAX__;
        max_price_value = __DBL_MIN__;

        while (trades[ind].created_at < query.start_time_point) ind++;

        while (trades[ind].created_at < query.end_time_point && trades[ind].created_at < offset + query.resolution) {
            double price_value = trades[ind].price.price * std::pow(10, trades[ind].price.price_exponent);

            if (price_value < min_price_value) {
                min_price_value = price_value;
                min_price = trades[ind].price;
            } 
            if (price_value > max_price_value) {
                max_price_value = price_value;
                max_price = trades[ind].price;
            }
        }

        result.push_back({min_price, max_price});
    }

    return result;
}


namespace ranges = std::ranges;

std::vector<TradeData> Executor::send_raw_data(TradeDataQuery &query)
{
  auto it_start = std::ranges::lower_bound(trades, query.start_time_point, {}, &TradeData::created_at);
  auto it_end = std::ranges::lower_bound(trades, query.end_time_point, {}, &TradeData::created_at);

  return std::vector<TradeData>(it_start, it_end);
}
