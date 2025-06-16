#ifndef QUERY_H
#define QUERY_H

#include <cstdint>
/**
 * @struct TradeDataQuery
 * @brief Represents a query for trade data within a specified time range and
 * resolution.
 *
 * @param symbol_id         The unique identifier for the symbol.
 * @param start_time_point  The start of the time range for the query (in
 * nanoseconds since epoch).
 * @param end_time_point    The end of the time range for the query (in
 * nanoseconds since epoch).
 * @param resolution        The data resolution i.e., 0 indicates raw data feed.
 * @param metrics           Bit flags specifying which trade metrics to include
 * in the result.
 */
struct TradeDataQuery {
  uint32_t symbol_id;
  uint64_t start_time_point;
  uint64_t end_time_point;
  uint64_t resolution;
  uint64_t metrics;
};

/**
 * @struct Price
 * @brief Represents a price with its associated exponent for decimal scaling.
 */
#pragma pack(push, 1)
struct Price {
  uint32_t price;
  int8_t price_exponent;
};
#pragma pack(pop)
/**
 * @struct Quantity
 * @brief Represents a quantity with its associated exponent for decimal
 * scaling.
 */
#pragma pack(push, 1)
struct Quantity {
  uint32_t quantity;
  int8_t quantity_exponent;
};
#pragma pack(pop)
/**
 * @struct trade_data
 * @brief struct as defined in the presentation
 */
#pragma pack(push, 2)
struct TradeData {
  uint32_t symbol_id;
  uint64_t created_at;  // std::chrono::time_point in nanoseconds
  uint64_t trade_id;

  Price price;
  Quantity quantity;

  uint8_t taker_side;  // 1 = ask, 2 = bid
  uint8_t padding_;    // just extra padding to make it 32 bytes
};
#pragma pack(pop)
/**
 * @struct Result
 * @brief struct with fields for the lowest and highest prices for a specific
 * start time.
 *
 * @param start_time
 * @param lowest_price
 * @param highest_price
 */
struct Result {
  uint64_t start_time;
  Price lowest_price;
  Price highest_price;
};

#endif
