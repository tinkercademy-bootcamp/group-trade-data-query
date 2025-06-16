#include <iostream>
#include <string>
#include <thread>
#include <chrono>

int32_t main() {
    std::string input;
    while (std::getline(std::cin, input)) {
        std::this_thread::sleep_for(std::chrono::seconds(5));  // Sleep for 5 seconds
        std::cout << input << "_processed" << std::endl;
    }
    return 0;
}
