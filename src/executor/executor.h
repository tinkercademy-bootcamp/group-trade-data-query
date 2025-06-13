#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <memory>
#include <string>
#include <vector>

#include "../utils/query.h"

class Executor {
public:
    struct price {
      uint32_t price;
      int8_t price_exponent;
    };
    // function to find lowest and highest prices with argument as a struct query uint32_t Symbol Id, uint64_t start_time_point; uint64_t end_time_point; uint64_t resolution;   // 0 to mean raw data feed; uint64_t metrics;      // bit flag returning the metrics as std::vector with std::pair
    std::vector<std::pair<price, price>> lowest_and_highest_prices(TradeDataQuery& query);
    // fucntion to return the raw_data when the resolution is 0, i.e., the vetcor of structs just called
    std::vector<TradeDataQuery> send_raw_data (TradeDataQuery& query);


private:
    
};

#endif // EXECUTOR_H