#include "executor.h"
#include "../../processed_data/preproc.h"
#include "../utils/query.h"
#include <cmath>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <ranges>
#include <numeric>

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


    // result = std::vector<Result>(num_buckets);

    double min_price_value = __DBL_MAX__, max_price_value = __DBL_MIN__;
    uint64_t bucket_start_time = query.start_time_point;
    uint64_t ind = 0;

    for (uint64_t offset = query.start_time_point; offset < query.end_time_point; offset += query.resolution) {
        Price min_price = {0, 0};  // Reset min price
        Price max_price = {0, 0};  // Reset max price
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
            ind++;  // Move to the next trade
            if (ind >= trades.size()) {
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
        if (ind >= trades.size()) {
            break;  // No more trades to process
        }
    }

    return result;
}


std::vector<StdDevResult> Executor::price_standard_deviation(const TradeDataQuery& query) {
    std::vector<StdDevResult> result;

    if (trades.empty()) {
        std::cout << "[Executor] No trades available.\n";
        return result;
    }

    if ((query.metrics & (1 << 1)) == 0) {
        std::cout << "[Executor] Bit 1 (std deviation) not set. Skipping computation.\n";
        return result;
    }

    assert(query.resolution > 0);
    uint64_t ind = 0;
    uint64_t bucket_start_time = query.start_time_point;

    for (uint64_t offset = query.start_time_point;
         offset < query.end_time_point;
         offset += query.resolution) {

        std::vector<double> prices;

        
        while (ind < trades.size() && trades[ind].created_at < query.start_time_point) 
            ind++;

        
        while (ind < trades.size() &&
               trades[ind].created_at < query.end_time_point &&
               trades[ind].created_at < offset + query.resolution) {

            double price_val = trades[ind].price.price * std::pow(10, trades[ind].price.price_exponent);
            prices.push_back(price_val);
            ind++;
        }

        double stddev = 0.0;
        if (!prices.empty()) {
            double mean = std::accumulate(prices.begin(), prices.end(), 0.0) / prices.size();
            double sum_sq = 0.0;
            for (double p : prices) {
                sum_sq += (p - mean) * (p - mean);
            }
            stddev = std::sqrt(sum_sq / prices.size());
        }

        
        Price std_price = {0, 0};
        if (stddev > 0.0) {
            int exponent = 0;
            while (stddev >= 10.0) {
                stddev /= 10.0;
                exponent++;
            }
            while (stddev < 1.0 && stddev > 0.0) {
                stddev *= 10.0;
                exponent--;
            }
            std_price.price = static_cast<uint32_t>(stddev * 1000);  
            std_price.price_exponent = exponent - 3;
        }

        result.push_back({bucket_start_time, std_price});
        bucket_start_time += query.resolution;

        if (ind >= trades.size()) break;
    }

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

void print_stddev_results(const std::vector<StdDevResult>& results) {
    for (const auto& res : results) {
        std::cout << "Time: " << res.start_time << ", "
                  << "Std Dev: " << res.std_dev_price.price << "e"
                  << static_cast<int>(res.std_dev_price.price_exponent) << "\n";
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <csv-file>\n";
        return 1;
    }
    std::string filename = argv[1];
    auto trades = parse_csv(filename);

    Executor executor(trades);
    std::cout << "Parsed " << trades.size() << " trades from " << filename << std::endl;
    TradeDataQuery query = {
        .symbol_id = 1,
        .start_time_point = 1747267200000000000,
        .end_time_point =   1747267200001855159, // 1 second in nanoseconds
        .resolution = 1000000000000000000, // 100 milliseconds
        .metrics = 2 // Enable min/max
    };
    auto results = executor.lowest_and_highest_prices(query);
    std::cout << "Computed " << results.size() << " results for lowest and highest prices.\n";
    for (const auto& res : results) {
        std::cout << "Start Time: " << res.start_time << ", "
                  << "Lowest Price: " << res.lowest_price.price << " * 10^" 
                  << static_cast<int>(res.lowest_price.price_exponent) << ", "
                  << "Highest Price: " << res.highest_price.price << " * 10^" 
                  << static_cast<int>(res.highest_price.price_exponent) << "\n";
    }
    auto raw_data = executor.send_raw_data(query);
    std::cout << "Retrieved " << raw_data.size() << " raw trades.\n";
    for (const auto& trade : raw_data) {
        std::cout << "Trade ID: " << trade.trade_id << ", "
                  << "Symbol ID: " << trade.symbol_id << ", "
                  << "Created At: " << trade.created_at << ", "
                  << "Price: " << trade.price.price << " * 10^" 
                  << static_cast<int>(trade.price.price_exponent) << ", "
                  << "Quantity: " << trade.quantity.quantity << " * 10^" 
                    << static_cast<int>(trade.quantity.quantity_exponent) << "\n";
    }
    auto stddev_results = executor.price_standard_deviation(query);
    std::cout << "Computed " << stddev_results.size() << " results for price standard deviation.\n";
    print_stddev_results(stddev_results);
    
    std::cout << "Executor operations completed successfully.\n";

    return 0;
}
