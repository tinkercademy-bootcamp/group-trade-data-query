#include <cstdint>

struct TradeDataQuery {
  uint32_t symbol_id;
  uint64_t start_time_point;
  uint64_t end_time_point;
  uint64_t resolution;   // 0 to mean raw data feed
  uint64_t metrics;      // bit flag to indicate what they want
};