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
    std::vector<std::pair<price, price>> lowest_and_highest_prices(TradeDataQuery& query);
    std::vector<TradeData> send_raw_data(TradeDataQuery& query);


private:
    
};

#endif // EXECUTOR_H