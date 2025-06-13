#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <memory>
#include <string>
#include <vector>

#include "../utils/trades.h"

/**
 * @class Executor
 * @brief Handles execution of trade data queries including price metrics and raw data retrieval.
 */
class Executor {
public:    
    std::vector<std::pair<Price, Price>> lowest_and_highest_prices(const std::vector<TradeData>& trades, const TradeDataQuery& query);
    std::vector<TradeData> send_raw_data(TradeDataQuery& query);


private:
    std::vector<TradeData>& trades;
};

#endif // EXECUTOR_H