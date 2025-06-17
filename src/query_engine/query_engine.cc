#include "query_engine.h"
// #include "../../processed_data/preproc.h"
#include "../utils/query.h"
#include <cmath>
#include <cstdint>
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
	SIZE[0] = 2*sizeof(Price); // Lowest and highest prices
	SIZE[26] = 2*sizeof(Price); // Mean price
	SIZE[33] = 2*sizeof(Quantity); // Total quantity
}

Query_engine::~Query_engine() {
	if (data.is_open()) {
		data.close();
	}
}

bool Query_engine::read_trade_data(uint64_t ind, TradeData& trade) {
  data.seekg(ind * sizeof(TradeData), std::ios::beg);
  data.read(reinterpret_cast<char *>(&trade), sizeof(TradeData));
  return true; // If we reach here, it means reading failed
}

uint64_t int_ceil(uint64_t x, uint64_t y){
  return (x/y) + (x%y != 0);
}

Price double_to_price(float64_t value) {
	Price price;
	if (value == 0.0) {
		price.price = 0;
		price.price_exponent = 0;
		return price;
	}
	
	int8_t exponent = static_cast<int8_t>(std::floor(std::log10(std::abs(value))));
	float64_t mantissa = value / std::pow(10, exponent);
	
	// Ensure mantissa is within the range of uint32_t
	if (mantissa < 0 || mantissa > std::numeric_limits<uint32_t>::max()) {
		throw std::overflow_error("Mantissa out of bounds for Price struct");
	}
	
	price.price = static_cast<uint32_t>(std::round(mantissa));
	price.price_exponent = exponent;
	
	return price;
}

Quantity double_to_quantity(float64_t value) {
	Quantity quantity;
	if (value == 0.0) {
		quantity.quantity = 0;
		quantity.quantity_exponent = 0;
		return quantity;
	}
	
	int8_t exponent = static_cast<int8_t>(std::floor(std::log10(std::abs(value))));
	float64_t mantissa = value / std::pow(10, exponent);
	
	// Ensure mantissa is within the range of uint32_t
	if (mantissa < 0 || mantissa > std::numeric_limits<uint32_t>::max()) {
		throw std::overflow_error("Mantissa out of bounds for Quantity struct");
	}
	
	quantity.quantity = static_cast<uint32_t>(std::round(mantissa));
	quantity.quantity_exponent = exponent;
	
	return quantity;
}

std::vector<char> Query_engine::aggregator(const TradeDataQuery& query){
  std::vector<char> res;

	TradeData trade;

  uint64_t outer_page_table_start_index = std::ranges::lower_bound(*outer_page_table, query.start_time_point) - outer_page_table->begin();
  if(outer_page_table_start_index != 0) outer_page_table_start_index -= 1;
  std::cout << "outer_page_table_start_index = " << outer_page_table_start_index << std::endl;

  uint64_t outer_page_table_end_index = std::ranges::lower_bound(*outer_page_table, query.end_time_point) - outer_page_table->begin();
  if(outer_page_table_end_index != 0) outer_page_table_end_index -= 1;

  // for(uint32_t idx = outer_page_table_start_index; idx <= outer_page_table_end_index; idx++) {
  //   read_inner_page_table(idx);
  // }

	uint64_t inner_page_table_start_index = std::lower_bound(inner_page_table->begin() + (outer_page_table_start_index << 9), inner_page_table->begin() + (outer_page_table_start_index << 9) + 1, query.start_time_point) - inner_page_table->begin();
	if(inner_page_table_start_index != 0) inner_page_table_start_index -= 1;
	std::cout << "inner_page_table_start_index = " << inner_page_table_start_index << std::endl;

	uint64_t inner_page_table_end_index = std::lower_bound(inner_page_table->begin() + (outer_page_table_end_index << 9), inner_page_table->begin() + (outer_page_table_end_index << 9) + 1, query.end_time_point) - inner_page_table->begin();
	if(inner_page_table_end_index != 0) inner_page_table_end_index -= 1;

	data.seekg((inner_page_table_start_index << 17) * sizeof(TradeData), std::ios::beg);
	uint64_t open_page_size = std::min((uint64_t)open_page_data.size(), trades_size - (inner_page_table_start_index << 17));

	// Read the data into open_page_data
	data.read(reinterpret_cast<char*>(open_page_data.data()), open_page_size * sizeof(TradeData));

	uint64_t page_offset = std::lower_bound(open_page_data.begin(), open_page_data.begin() + open_page_size, query.start_time_point, 
																		 [](const TradeData& trade, uint64_t time) {
																				 return trade.created_at < time;
																		 }) - open_page_data.begin();
	TradeData trade;
	uint32_t size = 0;
	size += sizeof(uint64_t); // Start time
	for (int8_t i = 0; i < 64; i++) {
		size += SIZE[i];
	}
	char buffer[size];
	uint32_t buffer_offset = 0;
	uint64_t resolution_start = query.start_time_point;
	uint64_t resolution_end = resolution_start + query.resolution;
	uint64_t num_of_trades_in_resolution = 0;

	float64_t price_mean = 0;
	float64_t quantity_sum = 0;

	while (page_offset < open_page_size && open_page_data[page_offset].created_at < query.end_time_point) {
		trade = open_page_data[page_offset];

		if (trade.created_at >= resolution_end) {
			buffer_offset = 0;
			*reinterpret_cast<uint64_t*>(buffer + buffer_offset) = resolution_start;

			if (query.metrics & (1 << 0)) buffer_offset += sizeof(uint64_t) + 2 * sizeof(Price);

			if (query.metrics & (1 << 26)) {
				*reinterpret_cast<Price*>(buffer + buffer_offset) = double_to_price(price_mean);
				buffer_offset += sizeof(Price);
				*reinterpret_cast<Price*>(buffer + buffer_offset) = double_to_price(price_mean);
				buffer_offset += sizeof(Price);
			}
			if (query.metrics & (1 << 33)) {
				*reinterpret_cast<Quantity*>(buffer + buffer_offset) = double_to_quantity(quantity_sum);
				buffer_offset += sizeof(Quantity);
			}
			res.insert(res.end(), buffer, buffer + buffer_offset);

			if (trade.created_at >= query.end_time_point) {
				break; // No more trades in the range
			}

			resolution_start = query.start_time_point + ((trade.created_at - query.start_time_point) / query.resolution) * query.resolution;
			resolution_end = resolution_start + query.resolution;
			*reinterpret_cast<uint64_t*>(buffer) = resolution_start;
			uint32_t offset = sizeof(uint64_t);
			if (query.metrics & (1 << 0)) {
				*reinterpret_cast<Price*>(buffer + offset) = trade.price;
				offset += sizeof(Price);
				*reinterpret_cast<Price*>(buffer + offset) = trade.price;
				offset += sizeof(Price);
			}
			if (query.metrics & (1 << 26)) {
				price_mean = 0;
			}
			num_of_trades_in_resolution = 0;
		}
		
		buffer_offset = sizeof(uint64_t);
		if (query.metrics & (1 << 0)) {
			if (trade.price <= *reinterpret_cast<Price*>(buffer + buffer_offset)) {
				*reinterpret_cast<Price*>(buffer + buffer_offset) = trade.price;
			}
			buffer_offset += sizeof(Price);
			if (!(trade.price <= *reinterpret_cast<Price*>(buffer + buffer_offset))) {
				*reinterpret_cast<Price*>(buffer + buffer_offset) = trade.price;
			}
			buffer_offset += sizeof(Price);
		}

		if (query.metrics & (1 << 26)) {
			price_mean *= num_of_trades_in_resolution;
			price_mean += trade.price.price * std::pow(10.0, trade.price.price_exponent);
			price_mean /= (num_of_trades_in_resolution + 1);
			num_of_trades_in_resolution++;
		}

		if (query.metrics & (1 << 33)) {
			quantity_sum += trade.quantity.quantity * std::pow(10.0, trade.quantity.quantity_exponent);
		}

		page_offset++;

		if (page_offset >= open_page_size) {
			// Read the next page of trades
			open_page_size = std::min((uint64_t)open_page_data.size(), trades_size - (inner_page_table_start_index << 17) - page_offset);
			data.read(reinterpret_cast<char*>(open_page_data.data()), open_page_size * sizeof(TradeData));
			page_offset = 0;
		}
	}
		


  // for (uint64_t start_time = query.start_time_point ; start_time < query.end_time_point; start_time += query.resolution) {
  //   uint64_t end_time = start_time + query.resolution;
  //   if (end_time > query.end_time_point) {
  //     end_time = query.end_time_point;
  //   }
  //   for (int i = 0; i < 8; i++) {
  //     if (chooser & (1 << i)) {
  //       switch (i) {
  //         case 0: { // min and max price
  //           auto [min_price, max_price] = min_max_price_in_range(start_time, end_time);
  //           // append min_price price (4 bytes, little-endian)
  //           {
  //             const char* ptr = reinterpret_cast<const char*>(&min_price.price);
  //             res.insert(res.end(), ptr, ptr + sizeof(min_price.price));
  //             // append exponent (1 byte)
  //             res.push_back(static_cast<char>(min_price.price_exponent));
  //             // append max_price price (4 bytes)
  //             ptr = reinterpret_cast<const char*>(&max_price.price);
  //             res.insert(res.end(), ptr, ptr + sizeof(max_price.price));
  //             // append exponent (1 byte)
  //             res.push_back(static_cast<char>(max_price.price_exponent));
  //           }
  //           break;
  //         }
  //         case 1: { // mean price
  //           Price mean_price = mean_price_in_range(start_time, end_time);
  //           {
  //             const char* ptr = reinterpret_cast<const char*>(&mean_price.price);
  //             res.insert(res.end(), ptr, ptr + sizeof(mean_price.price));
  //             res.push_back(static_cast<char>(mean_price.price_exponent));
  //           }
  //           break;
  //         }
  //         case 2: { // total quantity
  //           Quantity total_qty = total_quantity_in_range(start_time, end_time);
  //           {
  //             const char* ptr = reinterpret_cast<const char*>(&total_qty.quantity);
  //             res.insert(res.end(), ptr, ptr + sizeof(total_qty.quantity));
  //             res.push_back(static_cast<char>(total_qty.quantity_exponent));
  //           }
  //           break;
  //         }
  //       }
  //     }
  //   }
  // }
  return res;
}

std::pair<Price, Price> Query_engine::min_max_price_in_range(
    uint64_t start_time,
    uint64_t end_time)
{
    assert(end_time > start_time);
    TradeData trade;

    // 1) Binary-search for the first trade ≥ start_time
    uint64_t left  = 0;
    uint64_t right = trades_size - 1;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < start_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t start_idx = left;

    // 2) Binary-search for the first trade ≥ end_time, then back up one
    left  = start_idx;
    right = trades_size;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < end_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t end_idx = (right == 0 ? 0 : right - 1);

    // 3) Scan [start_idx..end_idx] to compute min/max
    bool first     = true;
    double min_val = 0, max_val = 0;
    Price  min_p{}, max_p{};
    for (uint64_t i = start_idx; i <= end_idx; ++i) {
        read_trade_data(i, trade);
        double v = trade.price.price * std::pow(10.0, trade.price.price_exponent);
        if (first || v < min_val) {
            min_val = v;
            min_p   = trade.price;
        }
        if (first || v > max_val) {
            max_val = v;
            max_p   = trade.price;
        }
        first = false;
    }

    return { min_p, max_p };
}

Quantity Query_engine::total_quantity_in_range(
    uint64_t start_time,
    uint64_t end_time)
{
    assert(end_time > start_time);
    TradeData trade;

    // 1) Find first trade ≥ start_time
    uint64_t left  = 0;
    uint64_t right = trades_size - 1;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < start_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t start_idx = left;

    // 2) Find first trade ≥ end_time, back up one to get last < end_time
    left  = start_idx;
    right = trades_size;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < end_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t end_idx = (right == 0 ? 0 : right - 1);

    // 3) Sum all quantities in [start_idx..end_idx] as a double
    double total_val = 0.0;
    for (uint64_t i = start_idx; i <= end_idx; ++i) {
        read_trade_data(i, trade);
        total_val +=
            trade.quantity.quantity *
            std::pow(10.0, trade.quantity.quantity_exponent);
    }

    // 4) Convert the accumulated double back into a Quantity struct
    int8_t exp = 0;

    // Scale UP if total_val is very small (to preserve precision)
    while (total_val > 0 && total_val < 1000000000.0 && exp > -18) {
        total_val *= 10.0;
        --exp;
    }

    // Scale DOWN if total_val is too large for uint32_t
    while (total_val > static_cast<double>(std::numeric_limits<uint32_t>::max())) {
        total_val /= 10.0;
        ++exp;
    }

    uint32_t mantissa = static_cast<uint32_t>(std::round(total_val));
    return Quantity{ mantissa, exp };
}

// Computes the volume‐weighted average price between start_time (inclusive)
// and end_time (exclusive). Returns the result as a packed `Price` struct.
Price Query_engine::mean_price_in_range(
    uint64_t start_time,
    uint64_t end_time)
{
    assert(end_time > start_time);
    TradeData trade;

    // 1) Binary‐search for the first trade ≥ start_time
    uint64_t left  = 0;
    uint64_t right = trades_size - 1;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < start_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t start_idx = left;

    // 2) Binary‐search for the first trade ≥ end_time, then back up one
    left  = start_idx;
    right = trades_size;
    while (left < right) {
        uint64_t mid = left + ((right - left) >> 1);
        read_trade_data(mid, trade);
        if (trade.created_at < end_time)
            left = mid + 1;
        else
            right = mid;
    }
    uint64_t end_idx = (right == 0 ? 0 : right - 1);

    // 3) Scan [start_idx..end_idx] to compute weighted mean
    double sum_pq = 0.0;  // sum of price * quantity
    double sum_q  = 0.0;  // sum of quantity
    for (uint64_t i = start_idx; i <= end_idx; ++i) {
        read_trade_data(i, trade);
        double price_val = trade.price.price *
                           std::pow(10.0, trade.price.price_exponent);
        double qty_val   = trade.quantity.quantity *
                           std::pow(10.0, trade.quantity.quantity_exponent);
        sum_pq += price_val * qty_val;
        sum_q  += qty_val;
    }

    // 4) Compute weighted mean
  double mean_val = (sum_q > 0.0 ? sum_pq / sum_q : 0.0);

  // 5) Convert the double mean back into a Price struct
  int8_t exp = 0;

  // Scale UP if mean_val is very small (to preserve precision)
  while (mean_val > 0 && mean_val < 1000000000.0 && exp > -18) {
      mean_val *= 10.0;
      --exp;
  }

  // Scale DOWN if mean_val is too large for uint32_t
  while (mean_val > static_cast<double>(std::numeric_limits<uint32_t>::max())) {
      mean_val /= 10.0;
      ++exp;
  }

  uint32_t mantissa = static_cast<uint32_t>(std::round(mean_val));
  return Price{ mantissa, exp };
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
