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
 * @file query_engine.h
 * @brief Declaration of the Query_engine class for processing trade data queries.
 */

/**
 * @class Query_engine
 * @brief Handles execution of trade data queries including price metrics and raw data retrieval.
 *
 * This class provides methods to aggregate trade data, compute price/quantity metrics,
 * and retrieve raw trade data from a binary file.
 */
class Query_engine {
public:
  /**
   * @brief Constructs a Query_engine and opens the trade data file.
   */
  Query_engine();

  /**
   * @brief Destructor for Query_engine. Closes the trade data file if open.
   */
  ~Query_engine();

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

  /**
   * @brief The number of trades in the loaded data file.
   */
  uint64_t trades_size;

  /**
   * @brief Reads a TradeData record at the specified index from the file.
   * @param ind Index of the trade record.
   * @param trade Reference to TradeData struct to fill.
   * @return true if read was successful, false otherwise.
   * @throws std::runtime_error if reading fails.
   */
  bool read_trade_data(uint64_t ind, TradeData& trade);

  /**
   * @brief Aggregates trade data metrics over time intervals as specified by the query.
   * @param metric_list Bitmask indicating which metrics to compute.
   * @param query The TradeDataQuery specifying the time range and resolution.
   * @return A vector of bytes representing the aggregated results.
   */
  std::vector<char> aggregator(int8_t metric_list, const TradeDataQuery& query);

  /**
   * @brief Finds the minimum and maximum prices in a given time range.
   * @param start_time Start of the time range (inclusive).
   * @param end_time End of the time range (exclusive).
   * @return Pair of Price structs: (min_price, max_price).
   */
  std::pair<Price, Price> min_max_price_in_range(
    uint64_t start_time,
    uint64_t end_time);

  /**
   * @brief Computes the total quantity traded in a given time range.
   * @param start_time Start of the time range (inclusive).
   * @param end_time End of the time range (exclusive).
   * @return Quantity struct representing the total quantity.
   */
  Quantity total_quantity_in_range(
    uint64_t start_time,
    uint64_t end_time);

  /**
   * @brief Computes the volume-weighted average price in a given time range.
   * @param start_time Start of the time range (inclusive).
   * @param end_time End of the time range (exclusive).
   * @return Price struct representing the mean price.
   */
  Price mean_price_in_range(
    uint64_t start_time,
    uint64_t end_time);

private:
  /**
   * @brief File stream for reading trade data.
   */
  std::ifstream data;
  // Any Internal members can be added here
  // std::vector<TradeData> trades;
};

#endif
