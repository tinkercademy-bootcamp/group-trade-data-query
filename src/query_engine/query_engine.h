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
class Query_engine {
public:

  Query_engine();
  ~Query_engine(); // <-- declare destructor here
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
  uint64_t trades_size;
  bool read_trade_data(uint64_t ind, TradeData& trade);

  std::vector<char> aggregator(int8_t metric_list, const TradeDataQuery& query);

  void min_max_price_in_range(
    uint64_t start_time,
    uint64_t end_time, std::vector<char> &res)
  ;

  void total_quantity_in_range(
    uint64_t start_time,
    uint64_t end_time,
    std::vector<char> &res)
  ;

  void mean_price_in_range(
    uint64_t start_time,
    uint64_t end_time, std::vector<char> &res)
  ;

  double read_price_prefix_sum(uint64_t ind);
  double compute_prefix_sum(uint64_t l, uint64_t r);
  uint64_t file_lower_bound(uint64_t time, uint64_t l);

private:
  std::ifstream data;  // File stream for reading trade data
  std::ifstream price_prefix_sum_file;
  // Any Internal members can be added here
  // std::vector<TradeData> trades;
};

#endif