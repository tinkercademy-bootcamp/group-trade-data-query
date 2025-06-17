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
    
  Query_engine(const std::string& file, std::shared_ptr<std::vector<uint64_t> > outer_page_table,
    std::shared_ptr<std::vector<uint64_t> > inner_page_table);
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
  uint64_t trades_size;
  
  bool read_trade_data(uint64_t ind, TradeData& trade);

  /**
    * @brief Retrieves outer page table data (trade.created_at) for a particular index
    *
    * @param index The index for which the outer page table data needs to be retrieved
    * @return trade.created_at for the corresponding index 
    */
  uint64_t read_outer_page_table_data(uint64_t index);
  std::vector<char> aggregator(const TradeDataQuery& query);

  std::pair<Price, Price> min_max_price_in_range(
    uint64_t start_time,
    uint64_t end_time)
  ;

  Quantity total_quantity_in_range(
    uint64_t start_time,
    uint64_t end_time)
  ;

  Price mean_price_in_range(
    uint64_t start_time,
    uint64_t end_time)
  ;

private:
	int64_t open_page;
	std::vector<TradeData> open_page_data;

	int32_t SIZE[64] = {0};

  std::shared_ptr<std::vector<uint64_t> > outer_page_table;
  std::shared_ptr<std::vector<uint64_t> > inner_page_table;
  std::ifstream data;  // File stream for reading trade data
  // Any Internal members can be added here
  // std::vector<TradeData> trades;
};

#endif
