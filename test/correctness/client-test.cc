#include <iostream>
#include <fstream>
#include "../nlohmann/json.hpp"
#include <cassert>
#include <vector>
#include <array>
#include <chrono>
#include <algorithm>
#include <numeric>

#include "../../src/utils/query.h"
#include "../../src/client/client.h"
#include "../../src/utils/net/net.h"

using json = nlohmann::json;

std::vector<Result> parse_results(const nlohmann::json& json_array) {
    std::vector<Result> results;
    for (const auto& entry : json_array) {
        Result res;
        res.start_time = entry[0].get<uint64_t>();
        res.lowest_price.price = entry[1][0].get<uint32_t>();
        res.lowest_price.price_exponent = entry[1][1].get<int8_t>();
        res.highest_price.price = entry[2][0].get<uint32_t>();
        res.highest_price.price_exponent = entry[2][1].get<int8_t>();
        results.push_back(res);
    }
    return results;
}

bool compare_results(const std::vector<Result>& a, const std::vector<Result>& b) {
    if (a.size() != b.size()) return false;

    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].start_time != b[i].start_time) return false;

        if (a[i].lowest_price.price != b[i].lowest_price.price ||
            a[i].lowest_price.price_exponent != b[i].lowest_price.price_exponent)
            return false;

        if (a[i].highest_price.price != b[i].highest_price.price ||
            a[i].highest_price.price_exponent != b[i].highest_price.price_exponent)
            return false;
    }

    return true;
}

int main() {
    std::string server_ip = "127.0.0.1";
    int port = 8080;
    client::Client curr_client(port, server_ip);
    return 0;
  
    // std::ifstream file("new-tests.json");

    // if (!file) {
    //     std::cerr << "Could not open new-tests.json" << std::endl;
    //     return 1;
    // }

    // json j;
    // file >> j;
    // int test_count = 0;
    // std::vector<double> timings_ms;

    // for (const auto& test : j["tests"]) {
    //     ++test_count;

    //     TradeDataQuery tdq = {
    //         test["symbol_id"],
    //         test["start_time_point"],
    //         test["end_time_point"],
    //         test["resolution"],
    //         test["metrics"]
    //     };

    //     auto start_time = std::chrono::steady_clock::now();
    //     curr_client.send_message(tdq);
    //     auto output = curr_client.read_min_max();
    //     auto end_time = std::chrono::steady_clock::now();

    //     double elapsed_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    //     timings_ms.push_back(elapsed_ms);

    //     const auto& expected_output_json = test["output"];
    //     auto expected_output = parse_results(expected_output_json);

    //     if (compare_results(output, expected_output)) {
    //         std::cout << "Passed " << test_count << std::endl;
    //     } else {
    //         std::cout << "Failed " << test_count << std::endl;
    //         exit(1);
    //     }
    // }

    // // Compute statistics
    // double sum = std::accumulate(timings_ms.begin(), timings_ms.end(), 0.0);
    // double mean = sum / timings_ms.size();

    // std::sort(timings_ms.begin(), timings_ms.end());
    // double median = timings_ms.size() % 2 == 0
    //     ? (timings_ms[timings_ms.size() / 2 - 1] + timings_ms[timings_ms.size() / 2]) / 2.0
    //     : timings_ms[timings_ms.size() / 2];

    // double min_time = timings_ms.front();
    // double max_time = timings_ms.back();

    // std::cout << "\nTiming Statistics (in milliseconds):" << std::endl;
    // std::cout << "Mean:   " << mean << " ms" << std::endl;
    // std::cout << "Median: " << median << " ms" << std::endl;
    // std::cout << "Min:    " << min_time << " ms" << std::endl;
    // std::cout << "Max:    " << max_time << " ms" << std::endl;

    // return 0;
}
