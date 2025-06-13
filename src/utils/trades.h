#include <cstdint>

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