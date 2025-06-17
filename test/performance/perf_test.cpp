#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include "../nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std::chrono;

struct TestCase {
    std::string test_id;
    std::string input;
    std::string expected_output;
    std::string error_msg;
};

struct TestResult {
    std::string test_id;
    std::string client_id;
    bool passed;
    double response_time_ms;
    std::string actual_output;
    std::string error_message;
    high_resolution_clock::time_point start_time;
    high_resolution_clock::time_point end_time;
};

struct ClientMetrics {
    std::string client_id;
    int total_tests;
    int passed_tests;
    int failed_tests;
    double total_time_ms;
    double avg_time_ms;
    double min_time_ms;
    double max_time_ms;
    double p95_time_ms;
    double p99_time_ms;
    double accuracy_percent;
    std::vector<double> response_times;
};

std::mutex results_mutex;
std::vector<TestResult> all_results;

class PerformanceTestRunner {
private:
    std::string client_exec;
    std::vector<TestCase> test_cases;
    int num_clients;
    
public:
    PerformanceTestRunner(const std::string& exec_path, int clients) 
        : client_exec(exec_path), num_clients(clients) {}
    
    // Load test cases from JSON
    bool load_tests(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Failed to open test file: " << filename << std::endl;
                return false;
            }

            json j;
            file >> j;
            for (const auto& t : j["tests"]) {
                test_cases.push_back({
                    t["test_id"],
                    t["input"],
                    t["expected_output"],
                    t["error_msg"]
                });
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading tests: " << e.what() << std::endl;
            return false;
        }
    }
    
    // Trim trailing whitespace
    std::string trim(const std::string& s) {
        size_t end = s.find_last_not_of(" \n\r\t");
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }
    
    // Run a single test and measure performance
    TestResult run_single_test(const TestCase& test, const std::string& client_id) {
        TestResult result;
        result.test_id = test.test_id;
        result.client_id = client_id;
        result.passed = false;
        result.response_time_ms = 0.0;
        result.start_time = high_resolution_clock::now();
        
        std::string command = "sh -c 'printf \"" + test.input + "\\n\" | " + client_exec + "' 2>&1";
        
        FILE* client = popen(command.c_str(), "r");
        if (!client) {
            result.end_time = high_resolution_clock::now();
            result.response_time_ms = duration_cast<microseconds>(result.end_time - result.start_time).count() / 1000.0;
            result.error_message = "Could not start client (popen failed)";
        }

        std::ostringstream output_stream;
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), client)) {
            output_stream << buffer;
        }
        std::string full_output = output_stream.str();

        result.end_time = high_resolution_clock::now();
        result.response_time_ms = duration_cast<microseconds>(result.end_time - result.start_time).count() / 1000.0;

        std::string response = trim(full_output);
        std::string expected = trim(test.expected_output);
        result.actual_output = response;

        if ((response.empty() && expected.empty()) || response == expected) {
            result.passed = true;
        } else {
            result.error_message = test.error_msg;
            std::cout << "response: " << response << std::endl;
            std::cout << "expected: " << expected << std::endl;
        }

        pclose(client);
        return result;
    }
    
    // Run all tests for a single client
    void run_client_tests(const std::string& client_id) {
        std::cout << "[INFO] Client " << client_id << " starting tests..." << std::endl;
        
        for (const auto& test : test_cases) {
            TestResult result = run_single_test(test, client_id);
            
            {
                std::lock_guard<std::mutex> lock(results_mutex);
                all_results.push_back(result);
            }
            
            // Optional: Add small delay between tests to avoid overwhelming the system
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        std::cout << "[INFO] Client " << client_id << " completed all tests." << std::endl;
    }
    
    // Calculate percentile
    double calculate_percentile(std::vector<double>& times, double percentile) {
        if (times.empty()) return 0.0;
        
        std::sort(times.begin(), times.end());
        size_t index = static_cast<size_t>((percentile / 100.0) * (times.size() - 1));
        return times[index];
    }
    
    // Calculate metrics for a specific client
    ClientMetrics calculate_client_metrics(const std::string& client_id) {
        ClientMetrics metrics;
        metrics.client_id = client_id;
        metrics.total_tests = 0;
        metrics.passed_tests = 0;
        metrics.failed_tests = 0;
        metrics.total_time_ms = 0.0;
        metrics.min_time_ms = std::numeric_limits<double>::max();
        metrics.max_time_ms = 0.0;
        
        for (const auto& result : all_results) {
            if (result.client_id == client_id) {
                metrics.total_tests++;
                metrics.total_time_ms += result.response_time_ms;
                metrics.response_times.push_back(result.response_time_ms);
                
                if (result.passed) {
                    metrics.passed_tests++;
                } else {
                    metrics.failed_tests++;
                }
                
                metrics.min_time_ms = std::min(metrics.min_time_ms, result.response_time_ms);
                metrics.max_time_ms = std::max(metrics.max_time_ms, result.response_time_ms);
            }
        }
        
        if (metrics.total_tests > 0) {
            metrics.avg_time_ms = metrics.total_time_ms / metrics.total_tests;
            metrics.accuracy_percent = (static_cast<double>(metrics.passed_tests) / metrics.total_tests) * 100.0;
            
            // Calculate percentiles
            std::vector<double> times_copy = metrics.response_times;
            metrics.p95_time_ms = calculate_percentile(times_copy, 95.0);
            metrics.p99_time_ms = calculate_percentile(times_copy, 99.0);
        }
        
        return metrics;
    }
    
    // Run performance tests with multiple clients
    void run_performance_tests() {
        auto overall_start = high_resolution_clock::now();
        
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "PERFORMANCE TEST STARTED" << std::endl;
        std::cout << "Clients: " << num_clients << std::endl;
        std::cout << "Tests per client: " << test_cases.size() << std::endl;
        std::cout << "Total tests: " << (num_clients * test_cases.size()) << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        // Clear previous results
        {
            std::lock_guard<std::mutex> lock(results_mutex);
            all_results.clear();
        }
        
        // Start all client threads
        std::vector<std::thread> client_threads;
        for (int i = 0; i < num_clients; i++) {
            std::string client_id = "Client_" + std::to_string(i + 1);
            client_threads.emplace_back(&PerformanceTestRunner::run_client_tests, this, client_id);
        }
        
        // Wait for all clients to complete
        for (auto& thread : client_threads) {
            thread.join();
        }
        
        auto overall_end = high_resolution_clock::now();
        double overall_time_ms = duration_cast<microseconds>(overall_end - overall_start).count() / 1000.0;
        
        // Generate and display metrics
        generate_performance_report(overall_time_ms);
    }
    
    // Generate comprehensive performance report
    void generate_performance_report(double overall_time_ms) {
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "PERFORMANCE REPORT" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        // Overall metrics
        int total_tests = all_results.size();
        int total_passed = 0;
        int total_failed = 0;
        double total_response_time = 0.0;
        std::vector<double> all_response_times;
        
        for (const auto& result : all_results) {
            if (result.passed) total_passed++;
            else total_failed++;
            total_response_time += result.response_time_ms;
            all_response_times.push_back(result.response_time_ms);
        }
        
        double overall_accuracy = (static_cast<double>(total_passed) / total_tests) * 100.0;
        double avg_response_time = total_response_time / total_tests;
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "\nOVERALL METRICS:" << std::endl;
        std::cout << "├─ Total Tests: " << total_tests << std::endl;
        std::cout << "├─ Passed: " << total_passed << " (" << (static_cast<double>(total_passed)/total_tests)*100 << "%)" << std::endl;
        std::cout << "├─ Failed: " << total_failed << " (" << (static_cast<double>(total_failed)/total_tests)*100 << "%)" << std::endl;
        std::cout << "├─ Overall Accuracy: " << overall_accuracy << "%" << std::endl;
        std::cout << "├─ Total Execution Time: " << overall_time_ms << " ms" << std::endl;
        std::cout << "├─ Average Response Time: " << avg_response_time << " ms" << std::endl;
        std::cout << "├─ Min Response Time: " << *std::min_element(all_response_times.begin(), all_response_times.end()) << " ms" << std::endl;
        std::cout << "├─ Max Response Time: " << *std::max_element(all_response_times.begin(), all_response_times.end()) << " ms" << std::endl;
        std::cout << "├─ P95 Response Time: " << calculate_percentile(all_response_times, 95.0) << " ms" << std::endl;
        std::cout << "└─ P99 Response Time: " << calculate_percentile(all_response_times, 99.0) << " ms" << std::endl;
        
        // Per-client metrics
        std::cout << "\nPER-CLIENT METRICS:" << std::endl;
        std::cout << std::string(120, '-') << std::endl;
        std::cout << std::left << std::setw(12) << "Client"
                  << std::setw(10) << "Tests"
                  << std::setw(10) << "Passed"
                  << std::setw(10) << "Failed"
                  << std::setw(12) << "Accuracy%"
                  << std::setw(12) << "Avg(ms)"
                  << std::setw(12) << "Min(ms)"
                  << std::setw(12) << "Max(ms)"
                  << std::setw(12) << "P95(ms)"
                  << std::setw(12) << "P99(ms)" << std::endl;
        std::cout << std::string(120, '-') << std::endl;
        
        for (int i = 0; i < num_clients; i++) {
            std::string client_id = "Client_" + std::to_string(i + 1);
            ClientMetrics metrics = calculate_client_metrics(client_id);
            
            std::cout << std::left << std::setw(12) << metrics.client_id
                      << std::setw(10) << metrics.total_tests
                      << std::setw(10) << metrics.passed_tests
                      << std::setw(10) << metrics.failed_tests
                      << std::setw(12) << std::fixed << std::setprecision(1) << metrics.accuracy_percent
                      << std::setw(12) << std::fixed << std::setprecision(2) << metrics.avg_time_ms
                      << std::setw(12) << metrics.min_time_ms
                      << std::setw(12) << metrics.max_time_ms
                      << std::setw(12) << metrics.p95_time_ms
                      << std::setw(12) << metrics.p99_time_ms << std::endl;
        }
        
        // Test-specific metrics
        std::cout << "\nTEST-SPECIFIC METRICS:" << std::endl;
        std::cout << std::string(80, '-') << std::endl;
        
        // Group results by test_id
        std::map<std::string, std::vector<TestResult>> test_groups;
        for (const auto& result : all_results) {
            test_groups[result.test_id].push_back(result);
        }
        
        std::cout << std::left << std::setw(20) << "Test ID"
                  << std::setw(10) << "Runs"
                  << std::setw(10) << "Passed"
                  << std::setw(12) << "Success%"
                  << std::setw(12) << "Avg(ms)"
                  << std::setw(12) << "P95(ms)" << std::endl;
        std::cout << std::string(80, '-') << std::endl;
        
        for (const auto& [test_id, results] : test_groups) {
            int runs = results.size();
            int passed = 0;
            std::vector<double> times;
            
            for (const auto& result : results) {
                if (result.passed) passed++;
                times.push_back(result.response_time_ms);
            }
            
            double success_rate = (static_cast<double>(passed) / runs) * 100.0;
            double avg_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
            double p95_time = calculate_percentile(times, 95.0);
            
            std::cout << std::left << std::setw(20) << test_id
                      << std::setw(10) << runs
                      << std::setw(10) << passed
                      << std::setw(12) << std::fixed << std::setprecision(1) << success_rate
                      << std::setw(12) << std::fixed << std::setprecision(2) << avg_time
                      << std::setw(12) << p95_time << std::endl;
        }
        
        // Save results to CSV
        save_results_to_csv();
        
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "PERFORMANCE TEST COMPLETED" << std::endl;
        std::cout << "Results saved to: performance_results.csv" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
    }
    
    // Save detailed results to CSV
    void save_results_to_csv() {
        std::ofstream csv_file("performance_results.csv");
        csv_file << "client_id,test_id,passed,response_time_ms,actual_output,error_message\n";
        
        for (const auto& result : all_results) {
            csv_file << result.client_id << ","
                     << result.test_id << ","
                     << (result.passed ? "true" : "false") << ","
                     << std::fixed << std::setprecision(3) << result.response_time_ms << ","
                     << "\"" << result.actual_output << "\","
                     << "\"" << result.error_message << "\"\n";
        }
        
        csv_file.close();
    }
};

int main(int argc, char* argv[]) {
    // Default values
    std::string client_exec = "./../../build/dummy_client";
    std::string test_file = "./performance-tests.json";
    int num_clients = 1;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--clients" && i + 1 < argc) {
            num_clients = std::atoi(argv[++i]);
        } else if (arg == "--exec" && i + 1 < argc) {
            client_exec = argv[++i];
        } else if (arg == "--tests" && i + 1 < argc) {
            test_file = argv[++i];
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --clients N    Number of concurrent clients (default: 1)\n";
            std::cout << "  --exec PATH    Path to client executable (default: ./../../build/dummy_client)\n";
            std::cout << "  --tests FILE   Path to test file (default: ./performance-tests.json)\n";
            std::cout << "  --help         Show this help message\n";
            return 0;
        }
    }
    
    if (num_clients <= 0) {
        std::cerr << "Error: Number of clients must be positive" << std::endl;
        return 1;
    }
    
    PerformanceTestRunner runner(client_exec, num_clients);
    
    if (!runner.load_tests(test_file)) {
        return 1;
    }
    
    runner.run_performance_tests();
    
    return 0;
}
