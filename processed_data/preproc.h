#pragma once

#include <string>
#include <vector>
#include "../src/utils/query.h"

void print_trade(const TradeData& trade);
std::vector<TradeData> parse_csv(const std::string& filepath);
