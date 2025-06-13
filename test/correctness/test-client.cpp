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
#include "../nlohmann/json.hpp"

using json = nlohmann::json;

struct TestCase {
    std::string test_id;
    std::string input;
    std::string expected_output;
    std::string error_msg;
};

std::mutex cout_mutex;

// Load test cases from JSON
std::vector<TestCase> load_tests(const std::string& filename) {
    std::vector<TestCase> tests;
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Failed to open test file");

    json j;
    file >> j;
    for (const auto& t : j["tests"]) {
        tests.push_back({
            t["test_id"],
            t["input"],
            t["expected_output"],
            t["error_msg"]
        });
    }
    return tests;
}

// Trim trailing whitespace
std::string trim(const std::string& s) {
    size_t end = s.find_last_not_of(" \n\r\t");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
void run_test(const TestCase& test, const std::string& client_exec, int& passed_count) {
    std::string command = "echo \"" + test.input + "\" | " + client_exec + " 2>&1";
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "[DEBUG] Running test: " << test.test_id << "\n";
        std::cerr << "[DEBUG] Command: " << command << "\n";
    }

    FILE* client = popen(command.c_str(), "r");
    if (!client) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "[✗] " << test.test_id << ": Could not start client (popen failed)\n";
        std::cerr << "      errno: " << errno << " (" << strerror(errno) << ")\n";
        return;
    }

    char buffer[1024] = {0};
    if (!fgets(buffer, sizeof(buffer), client)) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "[✗] " << test.test_id << ": No response from client\n";
        pclose(client);
        return;
    }

    std::string response = trim(std::string(buffer));

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        if (response == test.expected_output) {
            std::cout << "[✓] " << test.test_id << "\n";
            passed_count++;
        } else {
            std::cout << "[✗] " << test.test_id << ": " << test.error_msg << "\n";
            std::cout << "    Expected: " << test.expected_output << "\n";
            std::cout << "    Got     : " << response << "\n";
        }
    }

    pclose(client);
}


int main() {
    // const std::string client_exec = "../../build/client";  // path to your compiled client
    const std::string client_exec = "./../../build/dummy_client";
    // const std::string client_exec = "./dummy_client";

    const std::string test_file = "./basic-tests.json";

    std::vector<TestCase> tests;
    try {
        tests = load_tests(test_file);
    } catch (const std::exception& e) {
        std::cerr << "Error loading tests: " << e.what() << std::endl;
        return 1;
    }

    int passed_count = 0;
    std::vector<std::thread> threads;

    // Run each test in a thread
    for (const auto& test : tests) {
        threads.emplace_back(run_test, test, client_exec, std::ref(passed_count));
    }

    // Wait for all threads
    for (auto& t : threads) t.join();

    std::cout << "\nSummary: " << passed_count << "/" << tests.size() << " tests passed.\n";
    return (passed_count == static_cast<int>(tests.size())) ? 0 : 1;

}
