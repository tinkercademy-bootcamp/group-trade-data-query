#pragma once
#include <cstdint>

// TradeDataQuery is a structure used to query trade data for a specific asset symbol over a defined time range.
struct TradeDataQuery {
  uint32_t symbol_id;           //Symbol ID of the asset
  uint64_t start_time_point;    // Start time point in nanoseconds
  uint64_t end_time_point;      // End time point in nanoseconds
  uint64_t resolution;          // 0 to mean raw data feed, in nanoseconds. It denotes the time interval of the data points.
  uint64_t metrics;             // bit flag which will be used to determine which metrics to return, for example min_max, mean, stddev, etc.
};