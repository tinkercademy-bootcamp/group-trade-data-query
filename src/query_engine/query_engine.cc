/**
 * @file query_engine.cc
 * @brief Implements the Query_engine class for processing trade data queries.
 *
 * Provides methods for aggregating trade data, computing price metrics,
 * and retrieving raw trade data from a binary file.
 */

#include "query_engine.h"
// #include "../../processed_data/preproc.h"
#include "../utils/query.h"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <ranges>

typedef double float64_t;

/**
 * @brief Constructs a Query_engine and opens the trade data file.
 */
Query_engine::Query_engine() {
  data.open("data/processed/trades-example.bin", std::ios::in | std::ios::binary);
  if (!data.is_open()) {
    std::cerr << "[Query_engine] Error: could not open trade data file.\n";
    return;
  }
  // Calculate the number of trades in the file
  trades_size = data.seekg(0, std::ios::end).tellg() / 31;
  data.seekg(0, std::ios::beg);
}

/**
 * @brief Destructor for Query_engine. Closes the trade data file if open.
 */
Query_engine::~Query_engine() {
  if (data.is_open()) {
    data.close();
  }
}

/**
 * @brief Reads a TradeData record at the specified index from the file.
 * @param ind Index of the trade record.
 * @param trade Reference to TradeData struct to fill.
 * @return true if read was successful, false otherwise.
 * @throws std::runtime_error if reading fails.
 */
bool Query_engine::read_trade_data(uint64_t ind, TradeData& trade) {
  data.seekg(ind * sizeof(TradeData), std::ios::beg);
  if (data.read(reinterpret_cast<char *>(&trade), sizeof(TradeData))) {
    return true;
  } else {
    throw std::runtime_error("[Query_engine] Error reading trade data at index " + std::to_string(ind));
  }
  return false; // If we reach here, it means reading failed
}

/**
 * @brief Helper function for integer ceiling division.
 */
uint64_t int_ceil(uint64_t x, uint64_t y){
  return (x/y) + (x%y != 0);
}

/**
 * @brief Aggregates trade data metrics over time intervals as specified by the query.
 * @param chooser Bitmask indicating which metrics to compute.
 * @param query The TradeDataQuery specifying the time range and resolution.
 * @return A vector of bytes representing the aggregated results.
 */
std::vector<char> Query_engine::aggregator(int8_t chooser, const TradeDataQuery& query){
  std::vector<char> res;
  for (uint64_t start_time = query.start_time_point ; start_time < query.end_time_point; start_time += query.resolution) {
    uint64_t end_time = start_time + query.resolution;
    if (end_time > query.end_time_point) {
      end_time = query.end_time_point;
    }
    for (int i = 0; i < 8; i++) {
      if (chooser & (1 << i)){
        switch (i) {
          case 0: { // min and max price
            auto [min_price, max_price] = min_max_price_in_range(start_time, end_time);
            // Append min_price (4 bytes) and exponent (1 byte)
            {
              const char* ptr = reinterpret_cast<const char*>(&min_price.price);
              res.insert(res.end(), ptr, ptr + sizeof(min_price.price));
              res.push_back(static_cast<char>(min_price.price_exponent));
              // Append max_price (4 bytes) and exponent (1 byte)
              ptr = reinterpret_cast<const char*>(&max_price.price);
              res.insert(res.end(), ptr, ptr + sizeof(max_price.price));
              res.push_back(static_cast<char>(max_price.price_exponent));
            }
            break;
          }
          case 1: { // mean price
            Price mean_price = mean_price_in_range(start_time, end_time);
            {
              const char* ptr = reinterpret_cast<const char*>(&mean_price.price);
              res.insert(res.end(), ptr, ptr + sizeof(mean_price.price));
              res.push_back(static_cast<char>(mean_price.price_exponent));
            }
            break;
          }
          case 2: { // total quantity
            Quantity total_qty = total_quantity_in_range(start_time, end_time);
            {
              const char* ptr = reinterpret_cast<const char*>(&total_qty.quantity);
              res.insert(res.end(), ptr, ptr + sizeof(total_qty.quantity));
              res.push_back(static_cast<char>(total_qty.quantity_exponent));
            }
            break;
          }
        }
      }
    }
  }
  return res;
}

/**
 * @brief Finds the minimum and maximum prices in a given time range.
 * @param start_time Start of the time range (inclusive).
 * @param end_time End of the time range (exclusive).
 * @return Pair of Price structs: (min_price, max_price).
 */
std::pair<Price, Price> Query_engine::min_max_price_in_range(
    uint64_t start_time,
    uint64_t end_time)
{
    assert(end_time > start_time);
    TradeData trade;

    // 1) Binary-search for the first trade ≥ start_time
    uint64_t left  = 0;
    uint64_t right = trades_size - 1;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < start_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t start_idx = left;

    // 2) Binary-search for the first trade ≥ end_time, then back up one
    left  = start_idx;
    right = trades_size;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < end_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t end_idx = (right == 0 ? 0 : right - 1);

    // 3) Scan [start_idx..end_idx] to compute min/max
    bool first     = true;
    double min_val = 0, max_val = 0;
    Price  min_p{}, max_p{};
    for (uint64_t i = start_idx; i <= end_idx; ++i) {
        read_trade_data(i, trade);
        double v = trade.price.price * std::pow(10.0, trade.price.price_exponent);
        if (first || v < min_val) {
            min_val = v;
            min_p   = trade.price;
        }
        if (first || v > max_val) {
            max_val = v;
            max_p   = trade.price;
        }
        first = false;
    }

    return { min_p, max_p };
}

/**
 * @brief Computes the total quantity traded in a given time range.
 * @param start_time Start of the time range (inclusive).
 * @param end_time End of the time range (exclusive).
 * @return Quantity struct representing the total quantity.
 */
Quantity Query_engine::total_quantity_in_range(
    uint64_t start_time,
    uint64_t end_time)
{
    assert(end_time > start_time);
    TradeData trade;

    // 1) Find first trade ≥ start_time
    uint64_t left  = 0;
    uint64_t right = trades_size - 1;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < start_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t start_idx = left;

    // 2) Find first trade ≥ end_time, back up one to get last < end_time
    left  = start_idx;
    right = trades_size;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < end_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t end_idx = (right == 0 ? 0 : right - 1);

    // 3) Sum all quantities in [start_idx..end_idx] as a double
    double total_val = 0.0;
    for (uint64_t i = start_idx; i <= end_idx; ++i) {
        read_trade_data(i, trade);
        total_val +=
            trade.quantity.quantity *
            std::pow(10.0, trade.quantity.quantity_exponent);
    }

    // 4) Convert the accumulated double back into a Quantity struct
    int8_t exp = 0;

    // Scale UP if total_val is very small (to preserve precision)
    while (total_val > 0 && total_val < 1000000000.0 && exp > -18) {
        total_val *= 10.0;
        --exp;
    }

    // Scale DOWN if total_val is too large for uint32_t
    while (total_val > static_cast<double>(std::numeric_limits<uint32_t>::max())) {
        total_val /= 10.0;
        ++exp;
    }

    uint32_t mantissa = static_cast<uint32_t>(std::round(total_val));
    return Quantity{ mantissa, exp };
}

/**
 * @brief Computes the volume-weighted average price in a given time range.
 * @param start_time Start of the time range (inclusive).
 * @param end_time End of the time range (exclusive).
 * @return Price struct representing the mean price.
 */
Price Query_engine::mean_price_in_range(
    uint64_t start_time,
    uint64_t end_time)
{
    assert(end_time > start_time);
    TradeData trade;

    // 1) Binary‐search for the first trade ≥ start_time
    uint64_t left  = 0;
    uint64_t right = trades_size - 1;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < start_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t start_idx = left;

    // 2) Binary‐search for the first trade ≥ end_time, then back up one
    left  = start_idx;
    right = trades_size;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < end_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t end_idx = (right == 0 ? 0 : right - 1);

    // 3) Scan [start_idx..end_idx] to compute weighted mean
    double sum_pq = 0.0;  // sum of price * quantity
    double sum_q  = 0.0;  // sum of quantity
    for (uint64_t i = start_idx; i <= end_idx; ++i) {
        read_trade_data(i, trade);
        double price_val = trade.price.price *
                           std::pow(10.0, trade.price.price_exponent);
        double qty_val   = trade.quantity.quantity *
                           std::pow(10.0, trade.quantity.quantity_exponent);
        sum_pq += price_val * qty_val;
        sum_q  += qty_val;
    }

    // 4) Compute weighted mean
    double mean_val = (sum_q > 0.0 ? sum_pq / sum_q : 0.0);

    // 5) Convert the double mean back into a Price struct
    int8_t exp = 0;

    // Scale UP if mean_val is very small (to preserve precision)
    while (mean_val > 0 && mean_val < 1000000000.0 && exp > -18) {
        mean_val *= 10.0;
        --exp;
    }

    // Scale DOWN if mean_val is too large for uint32_t
    while (mean_val > static_cast<double>(std::numeric_limits<uint32_t>::max())) {
        mean_val /= 10.0;
        ++exp;
    }

    uint32_t mantissa = static_cast<uint32_t>(std::round(mean_val));
    return Price{ mantissa, exp };
}

/**
 * @brief Retrieves all TradeData records matching the query's time range.
 * @param query The TradeDataQuery specifying the time window.
 * @return Vector of TradeData records within the specified range.
 */
std::vector<TradeData> Query_engine::send_raw_data(TradeDataQuery &query)
{
  std::vector<TradeData> trades;

  for (uint64_t ind = 0; ind < trades_size; ind++) {
    TradeData trade;
    if (!read_trade_data(ind, trade)) {
      std::cerr << "[Query_engine] Error reading trade data at index " << ind << ".\n";
      continue;
    }
    if (trade.created_at >= query.start_time_point && trade.created_at < query.end_time_point) {
      trades.push_back(trade);
    }
  }
  return trades;
}
