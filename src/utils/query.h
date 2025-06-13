#pragma once
#include <cstdint>

// TradeDataQuery is a structure used to query trade data for a specific asset symbol over a defined time range.
struct TradeDataQuery {
  uint32_t symbol_id;
  uint64_t start_time_point;
  uint64_t end_time_point;
  uint64_t resolution;   // 0 to mean raw data feed
  uint64_t metrics;      // bit flag
};

/**
 * @struct price
 * @brief Represents a price with its associated exponent for decimal scaling.
 */
struct price {
  uint32_t price;
  int8_t price_exponent;
};