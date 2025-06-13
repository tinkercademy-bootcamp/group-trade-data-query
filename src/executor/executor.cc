#include "executor.h"
#include "../utils/trades.h"
#include <cmath>
#include <iostream>
#include <cassert>

std::vector<std::pair<Price, Price>> Executor::lowest_and_highest_prices(
    const std::vector<TradeData>& trades, const TradeDataQuery& query) {

    std::vector<std::pair<Price, Price> > result;

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


    result = std::vector<std::pair<Price, Price>>(num_buckets, {Price{-1, 0}, Price{-1, 0}});

    uint64_t offset = 0;
    Price min_price = {-1, 0}, max_price = {-1, 0};
    double min_price_value = __DBL_MAX__, max_price_value = __DBL_MIN__;

    // Iterate over all trades and bucket them
    for (const TradeData& trade : trades) {
        if (trade.created_at < query.start_time_point || trade.created_at >= query.end_time_point) {
            continue;  // Skip trades outside query range
        }

        // We are determining which time bucket this trade belongs to
        offset = (trade.created_at - offset);
        if (offset >= query.resolution) {
            offset = (offset % query.resolution);
            result[static_cast<size_t>(offset / query.resolution)].first = min_price;
            result[static_cast<size_t>(offset / query.resolution)].second = max_price;
        }

        double price_value = trade.price.price * std::pow(10, trade.price.price_exponent) * trade.quantity.quantity * std::pow(10, trade.quantity.quantity_exponent);
        
        if (price_value > max_price_value) {
            max_price_value = price_value;
            max_price = trade.price;
        }
        if (price_value < min_price_value) {
            min_price_value = price_value;
            min_price = trade.price;
        }

        // size_t bucket_index = static_cast<size_t>(offset / num_buckets);

        // Calculating the real Price for comparison
    }

    return result;
}