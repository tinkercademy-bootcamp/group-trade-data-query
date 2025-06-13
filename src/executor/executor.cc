#include "executor.h"

namespace ranges = std::ranges;

std::vector<TradeData> Executor::send_raw_data(TradeDataQuery &query)
{
  auto it_start = std::ranges::lower_bound(trades, query.start_time_point, {}, &TradeData::created_at);
  auto it_end = std::ranges::lower_bound(trades, query.end_time_point, {}, &TradeData::created_at);

  return std::vector<TradeData>(it_start, it_end);
}
