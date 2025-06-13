#include "executor.h"
#include "../utils/trades.h"
#include <cmath>
#include <iostream>

std::vector<std::pair<price, price>> Executor::lowest_and_highest_prices(
    const std::vector<TradeData>& trades, const TradeDataQuery& query) {

    std::vector<std::pair<price, price> > result;

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

    // Calculate size of each time bucket based on resolution
    uint64_t num_buckets = (query.end_time_point - query.start_time_point + (query.resolution - 1)) / query.resolution;
    // uint64_t bucket_size = (query.end_time_point - query.start_time_point) / query.resolution;

    // Initialize result with sentinel values {-1, 0} indicating no data in bucket
    result.resize(num_buckets, {{-1, 0}, {-1, 0}});

    // Track lowest and highest trades for each bucket
    // std::vector<const TradeData*> lowest_in_bucket(num_buckets, nullptr);
    // std::vector<const TradeData*> highest_in_bucket(num_buckets, nullptr);

    // Iterate over all trades and bucket them
    for (const Trade& trade : trades) {
        if (trade.timestamp < query.start_time_point || trade.timestamp > query.end_time_point) {
            continue;  // Skip trades outside query range
        }

        // We are determining which time bucket this trade belongs to
        uint64_t offset = trade.timestamp - query.start_time_point;
        size_t bucket_index = static_cast<size_t>(offset / num_buckets);

        // Calculating the real price for comparison
        double real_price = trade.price * std::pow(10.0, trade.price_exponent);
        real_price = real_price * std::pow(trade.quantity, trade.quantity_exponent);

        if (real_price < result[bucket_index]->price * std::pow(10.0, result[bucket_index]->price_exponent)) {
            result[bucket_index] = {
                .price = trade.price,
                .price_exponent = trade.price_exponent
            };
        }
        if (real_price > result[bucket_index]->price * std::pow(10.0, result[bucket_index]->price_exponent)) {
            result[bucket_index] = {
                .price = trade.price,
                .price_exponent = trade.price_exponent
            };
        }
    }

    return result;
}

