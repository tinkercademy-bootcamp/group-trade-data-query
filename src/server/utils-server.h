#include "../utils/query.h" 
#include <vector>

// Item for the work queue
struct WorkItem {
    int32_t client_fd;
    TradeDataQuery query;
};

// Item for the results queue
struct ResultItem {
    int32_t client_fd;
    std::vector<char> res;
    // bool is_trade_data; // true if the result is TradeData, false if it is Result
    // std::vector<TradeData> trade_data_results;
    // std::vector<Result> resolution_results;
};