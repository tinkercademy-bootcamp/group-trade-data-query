#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <random>

int main() {
    std::string input;
    std::getline(std::cin, input);
    
    // Simulate some processing time (random between 1-50ms)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 50);
    int delay_ms = dis(gen);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    
    // Simply append "_processed" to the input
    std::cout << input << "_processed" << std::endl;
    
    return 0;
}