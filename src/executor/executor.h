#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <memory>
#include <string>
#include <vector>
#include <algorithm>

#include "../utils/query.h"

/**
 * @class Executor
 * @brief Handles execution of trade data queries including price metrics and raw data retrieval.
 */
class Executor
{
public:
    /**
     * @brief Computes the lowest and highest prices within the given query window.
     *
     * @param query  A TradeDataQuery specifying the symbol, time window, resolution, and metrics.
     * @return A vector of pairs of price structs representing the lowest and highest prices found.
     */
    std::vector<Result> lowest_and_highest_prices(TradeDataQuery &query);
    /**
     * @brief Retrieves raw trade data corresponding to the query parameters.
     *
     * @param query  A TradeDataQuery defining the criteria for raw data selection.
     * @return A vector of TradeData objects matching the query range and filters.
     */
    std::vector<TradeData> send_raw_data(TradeDataQuery &query);

private:
    // Any Internal members can be added here
    std::vector<TradeData> &trades;
};

#endif
