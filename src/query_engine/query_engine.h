#ifndef QUERY_ENGINE_H
#define QUERY_ENGINE_H

#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iterator>

#include "../utils/query.h"

/**
 * @class Query_engine
 * @brief Handles execution of trade data queries including price metrics and raw data retrieval.
 */
class Executor {
public:

  Executor();
  ~Executor() = default;
  /**
    * @brief Computes the lowest and highest prices within the given query window.
    *
    * @param query  A TradeDataQuery specifying the symbol, time window, resolution, and metrics.
    * @return A vector of pairs of price structs representing the lowest and highest prices found.
    */
  std::vector<Result> lowest_and_highest_prices(const TradeDataQuery &query);
  /**
    * @brief Retrieves raw trade data corresponding to the query parameters.
    *
    * @param query  A TradeDataQuery defining the criteria for raw data selection.
    * @return A vector of TradeData objects matching the query range and filters.
    */
  std::vector<TradeData> send_raw_data(TradeDataQuery &query);

private:
  std::ifstream data;  // File stream for reading trade data
  // Any Internal members can be added here
  // std::vector<TradeData> trades;
  uint64_t trades_size;
  bool read_trade_data(uint64_t ind, TradeData& trade);
};

#endif
