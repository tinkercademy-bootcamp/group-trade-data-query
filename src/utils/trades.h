#include <cstdint>
/**
 * @struct TradeDataQuery
 * @brief Represents a query for trade data within a specified time range and resolution.
 *
 * @param symbol_id         The unique identifier for the symbol.
 * @param start_time_point  The start of the time range for the query (in nanoseconds since epoch).
 * @param end_time_point    The end of the time range for the query (in nanoseconds since epoch).
 * @param resolution        The data resolution i.e., 0 indicates raw data feed.
 * @param metrics           Bit flags specifying which trade metrics to include in the result.
 */
struct TradeDataQuery {
  uint32_t symbol_id;
  uint64_t start_time_point;
  uint64_t end_time_point; 
  uint64_t resolution; 
  uint64_t metrics;     
};

/**
 * @struct price
 * @brief Represents a price with its associated exponent for decimal scaling.
 */
struct price {
  uint32_t price;
  int8_t price_exponent;
};

/**
* @struct quantity
* @brief Represents a quantity with its associated exponent for decimal scaling.
*/
struct quantity {
  uint32_t quantity;
  int8_t quantity_exponent;
};

/**
* @struct trade_data 
* @brief struct as defined in the presentation
*/
struct TradeData {
  uint32_t symbol_id;
  uint64_t created_at; // std::chrono::time_point in nanoseconds 
  uint64_t trade_id;

  
  price price;
  quantity quantity;

  uint8_t taker_side;       // 1 = ask, 2 = bid
};