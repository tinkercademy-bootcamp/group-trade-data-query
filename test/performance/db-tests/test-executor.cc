// #include <bits/stdc++.h>
#include "../../../src/query_engine/query_engine.h"
#include "../../../src/utils/query.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdint>

using namespace std;

unsigned long long rdtsc() {
    unsigned long long a, d;
    asm volatile("mfence");
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    a = (d << 32) | a;
    asm volatile("mfence");
    return a;
}

#define PAGE_SIZE 4096ULL
#define PAGEMAP_ENTRY 8

struct Region {
    uint64_t start;
    uint64_t end;
};

std::vector<Region> get_memory_regions(pid_t pid) {
    std::vector<Region> regions;
    std::ifstream maps("/proc/" + std::to_string(pid) + "/maps");
    std::string line;

    while (std::getline(maps, line)) {
        std::istringstream iss(line);
        std::string addr_range;
        if (!(iss >> addr_range)) continue;

        auto dash = addr_range.find('-');
        uint64_t start = std::stoull(addr_range.substr(0, dash), nullptr, 16);
        uint64_t end = std::stoull(addr_range.substr(dash + 1), nullptr, 16);
        regions.push_back({start, end});
    }

    return regions;
}

bool is_page_present(int pagemap_fd, uint64_t vaddr) {
    uint64_t offset = (vaddr / PAGE_SIZE) * PAGEMAP_ENTRY;
    if (lseek(pagemap_fd, offset, SEEK_SET) == (off_t)-1) {
        return false;
    }

    uint64_t entry;
    if (read(pagemap_fd, &entry, PAGEMAP_ENTRY) != PAGEMAP_ENTRY) {
        return false;
    }

    return entry & (1ULL << 63); // Present bit
}

uint64_t get_resident_memory(pid_t pid) {
    auto regions = get_memory_regions(pid);
    std::string pagemap_path = "/proc/" + std::to_string(pid) + "/pagemap";
    int fd = open(pagemap_path.c_str(), O_RDONLY);
    if (fd < 0) {
        perror("open pagemap");
        exit(1);
        return 0;
    }

    uint64_t total_present = 0;

    for (const auto& region : regions) {
        for (uint64_t addr = region.start; addr < region.end; addr += PAGE_SIZE) {
            if (is_page_present(fd, addr)) {
                total_present++;
            }
        }
    }

    close(fd);
    return total_present * PAGE_SIZE;
}

int main(int argc, char** argv) {
  std::cout << getpid() << std::endl;
  try {
    Que executor;
    uint64_t index = 0;
    uint64_t start_time, end_time;
    TradeData currentTrade;
    vector<uint64_t> differences;

    // std::cout << "Human test: input the integer you see on your screen" << std::endl;
    // uint16_t pid;
    // cin >> pid;
    // if (pid != getpid()) {
    //   cerr << "Error: PID mismatch. You are a robot" << endl;
    //   return 1;
    // }
    // if (argc < 2) {
    //   cerr << "Usage: " << argv[0] << " <number_of_trades>" << endl;
    //   return 1;
    // }

    int32_t num = atoi(argv[1]);
    
    cout << "Total trades: " << executor.trades_size << endl;
    cout << sizeof(TradeData) << " bytes per trade" << endl;
    
    while(index < num) {
      try {
        // start_time = rdtsc();
        executor.read_trade_data(index, currentTrade);
        // end_time = rdtsc();
        // differences.push_back(end_time - start_time);
      } catch (const exception& e) {
        cerr << "Error at index " << index << ": " << e.what() << endl;
        break;
      }
      index += 128;
      differences.push_back(get_resident_memory(getpid()));
    }
    
    cout << "Successfully processed " << differences.size() << " trades" << endl;
    
    FILE* f = fopen("differences.txt", "w");
    if (f != NULL) {
      for(int64_t i = 0; i < differences.size(); i++) {
        fprintf(f, "%ld %lu\n", i, differences[i]);
      }
      fclose(f);
      cout << "Results written to differences.txt" << endl;
    } else {
      cerr << "Failed to open differences.txt for writing" << endl;
    }
  } catch (const exception& e) {
    cerr << "Fatal error: " << e.what() << endl;
    return 1;
  }
  
  return 0;
}