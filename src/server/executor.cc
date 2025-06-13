#include "executor.h"
#include <cmath>
#include <iostream>

Executor::MinMaxResult Executor::lowest_and_highest_prices(
    const std::vector<Trade>& trades,
    const TradeDataQuery& query) {

    Executor::MinMaxResult result;
    result.min_price = {0, 0};
    result.max_price = {0, 0};

    if (trades.empty()) {
        std::cout << "[Executor] No trades available.\n";
        return result;
    }

    const Trade* min_trade = nullptr;
    const Trade* max_trade = nullptr;

    for (const Trade& trade : trades) {
        // Skip trades outside the query's time range
        if (trade.timestamp < query.start_time_point || trade.timestamp > query.end_time_point) {
            continue;
        }

        double current_price = trade.price * std::pow(10.0, trade.price_exponent);

        if (min_trade == nullptr) {
            min_trade = &trade;
        } else {
            double min_price = min_trade->price * std::pow(10.0, min_trade->price_exponent);
            if (current_price < min_price) {
                min_trade = &trade;
            }
        }

        if (max_trade == nullptr) {
            max_trade = &trade;
        } else {
            double max_price = max_trade->price * std::pow(10.0, max_trade->price_exponent);
            if (current_price > max_price) {
                max_trade = &trade;
            }
        }
    }

    // If we found valid trades within the time range, set the result
    if (min_trade && max_trade) {
        result.min_price.price = min_trade->price;
        result.min_price.price_exponent = min_trade->price_exponent;

        result.max_price.price = max_trade->price;
        result.max_price.price_exponent = max_trade->price_exponent;
    }

    return result;
}

