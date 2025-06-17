
#include "query_engine.h"
// #include "../../processed_data/preproc.h"
#include "../utils/query.h"
#include <cmath>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <ranges>
typedef double float64_t;

Query_engine::Query_engine(const std::string& file, 
                           std::shared_ptr<std::vector<uint64_t> > outer_page_table, 
                           std::shared_ptr<std::vector<uint64_t> > inner_page_table)
    : outer_page_table(outer_page_table), inner_page_table(inner_page_table) {
  data.open("data/processed/trades-" + file + ".bin", std::ios::in | std::ios::binary);
  if (!data.is_open()) {
    std::cerr << "[Query_engine] Error: could not open trade data file.\n";
    return;
  }
  trades_size = data.seekg(0, std::ios::end).tellg() / sizeof(TradeData);
  data.seekg(0, std::ios::beg);
  std::vector<TradeData> open_page_data(4 * 1024 * 128);
  open_page = -1;
}

Query_engine::~Query_engine() {
	if (data.is_open()) {
		data.close();
	}
}

bool Query_engine::read_trade_data(uint64_t ind, TradeData& trade) {
  data.seekg(ind * sizeof(TradeData), std::ios::beg);
  data.read(reinterpret_cast<char *>(&trade), sizeof(TradeData));
	std::cout << trade.symbol_id << " " 
    << trade.created_at << " "
    << trade.trade_id << " "
    << trade.price.price << " * 10^" 
    << static_cast<int32_t>(trade.price.price_exponent) << " "
    << trade.quantity.quantity << " * 10^" 
    << static_cast<int32_t>(trade.quantity.quantity_exponent) << " "
    << static_cast<int32_t>(trade.taker_side) << "\n";
  return false; // If we reach here, it means reading failed
}

uint64_t int_ceil(uint64_t x, uint64_t y){
  return (x/y) + (x%y != 0);
}

void read_inner_page_table(uint32_t outer_page_table_index) {
  // To be implemented
}

std::vector<Result> Query_engine::lowest_and_highest_prices(
  const TradeDataQuery& query) {

  std::vector<Result> result;

  // Early exit if no trades available
  // if (trades.empty()) {
  //     std::cout << "[Executor] No trades available.\n";
  //     return result;
  // }

  // Check if bit flag for min/max is enabled (bit 0)
  if ((query.metrics & (1 << 0)) == 0) {
    std::cout << "[Query_engine] Bit 0 (min/max) not set. Skipping computation.\n";
    return result;
  }

  assert(query.resolution > 0);

  double min_price_value = __DBL_MAX__, max_price_value = __DBL_MIN__;

  TradeData trade;

  uint32_t outer_page_table_start_index = std::ranges::lower_bound(*outer_page_table, query.start_time_point) - outer_page_table->begin();
  if(outer_page_table_start_index != 0) outer_page_table_start_index -= 1;
  std::cout << "outer_page_table_start_index = " << outer_page_table_start_index << std::endl;

  uint32_t outer_page_table_end_index = std::ranges::lower_bound(*outer_page_table, query.end_time_point) - outer_page_table->begin();
  if(outer_page_table_end_index != 0) outer_page_table_end_index -= 1;

  for(uint32_t idx = outer_page_table_start_index; idx <= outer_page_table_end_index; idx++) {
    read_inner_page_table(idx);
  }
	// if(open_page != -1) {

	// }

  uint64_t left_index = 0;
  // uint64_t right_index = trades_size - 1;
  // uint64_t middle_index = 0;
  // while(left_index < right_index) {
  //   middle_index = left_index + ((right_index - left_index) >> 1);
  //   read_trade_data(middle_index, trade);
  //   if(query.start_time_point <= trade.created_at) {
  //       right_index = middle_index;
  //   }
  //   else {
  //       left_index = middle_index + 1;
  //   }
  // }

  uint64_t ind = left_index;

  for (uint64_t offset = query.start_time_point; offset < query.end_time_point; offset += query.resolution) {
    if(ind >= trades_size) break;
        
    read_trade_data(ind,trade);

    if(offset < trade.created_at){
      uint64_t times = int_ceil(trade.created_at-offset+1, query.resolution)-1;
      offset += query.resolution*times;
    }

    Price min_price = {0, 0};  // Reset min price
    Price max_price = {0, 0};  // Reset max price
    min_price_value = __DBL_MAX__;
    max_price_value = __DBL_MIN__;

    while (ind < trades_size && read_trade_data(ind, trade) && trade.created_at < offset) ind++;
    uint64_t good_cnt = 0;

    while (ind < trades_size && read_trade_data(ind, trade) && 
          trade.created_at < query.end_time_point && trade.created_at >= offset && 
          trade.created_at < offset + query.resolution) {
      // std::cout << "Offset = " << offset << " " << std::endl;
      // std::cout << "Ind = " << ind << std::endl;
      // std::cout << "Create time = " << trade.created_at << std::endl;
      // std::cout << std::endl;

      double price_value = trade.price.price * std::pow(10, trade.price.price_exponent);
      ++good_cnt;

      if (price_value < min_price_value) {
        min_price_value = price_value;
        min_price = trade.price;
      } 
      if (price_value > max_price_value) {
        max_price_value = price_value;
        max_price = trade.price;
      }

      ind++;
    }

    // std::cout << "Offset = " << offset << std::endl;
    // std::cout << "Good = " << good_cnt << std::endl;

    if(good_cnt){
      result.push_back({
        offset,
        min_price,
        max_price
      });
    }
  }
  return result;
}

// namespace ranges = std::ranges;

std::vector<TradeData> Query_engine::send_raw_data(TradeDataQuery &query)
{
  std::vector<TradeData> trades;

  for (uint64_t ind = 0; ind < trades_size; ind++) {
    TradeData trade;
    if (!read_trade_data(ind, trade)) {
      std::cerr << "[Query_engine] Error reading trade data at index " << ind << ".\n";
      continue;
    }
    if (trade.created_at >= query.start_time_point && trade.created_at < query.end_time_point) {
      trades.push_back(trade);
    }
  }
  return trades;
}


// int32_t main(int32_t argc, char** argv) {
//     if (argc < 2) {
//         std::cerr << "Usage: " << argv[0] << " <csv-file>\n";
//         return 1;
//     }
//     std::string filename = argv[1];
//     // auto trades = parse_csv(filename);

//     Query_engine query_engine;
//     std::cout << "Parsed " << trades.size() << " trades from " << filename << std::endl;
//     TradeDataQuery query = {
//         .symbol_id = 1,
//         .start_time_point = 1747267200000000000,
//         .end_time_point =   1747267200001855159, // 1 second in nanoseconds
//         .resolution = 1000000000000000000, // 100 milliseconds
//         .metrics = 1 // Enable min/max
//     };
//     auto results = Query_engine.lowest_and_highest_prices(query);
//     std::cout << "Computed " << results.size() << " results for lowest and highest prices.\n";
//     for (const auto& res : results) {
//         std::cout << "Start Time: " << res.start_time << ", "
//                   << "Lowest Price: " << res.lowest_price.price << " * 10^" 
//                   << static_cast<int32_t>(res.lowest_price.price_exponent) << ", "
//                   << "Highest Price: " << res.highest_price.price << " * 10^" 
//                   << static_cast<int32_t>(res.highest_price.price_exponent) << "\n";
//     }
//     auto raw_data = Query_engine.send_raw_data(query);
//     std::cout << "Retrieved " << raw_data.size() << " raw trades.\n";
//     for (const auto& trade : raw_data) {
//         std::cout << "Trade ID: " << trade.trade_id << ", "
//                   << "Symbol ID: " << trade.symbol_id << ", "
//                   << "Created At: " << trade.created_at << ", "
//                   << "Price: " << trade.price.price << " * 10^" 
//                   << static_cast<int32_t>(trade.price.price_exponent) << ", "
//                   << "Quantity: " << trade.quantity.quantity << " * 10^" 
//                   << static_cast<int32_t>(trade.quantity.quantity_exponent) << "\n";
//     }
//     std::cout << "Query_engine operations completed successfully.\n";
    
//     return 0;
// }
