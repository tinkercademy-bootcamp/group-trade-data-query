#include <bits/stdc++.h>
#include "../../../src/query_engine/query_engine.h"
#include "../../../src/utils/query.h"

using namespace std;

unsigned long long rdtsc() {
    unsigned long long a, d;
    asm volatile("mfence");
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    a = (d << 32) | a;
    asm volatile("mfence");
    return a;
}

int main(int argc, char** argv) {
  std::cout << getpid() << std::endl;
  try {
    Query_engine executor;
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
        start_time = rdtsc();
        // executor.read_trade_data(index, currentTrade);
        end_time = rdtsc();
        // differences.push_back(end_time - start_time);
      } catch (const exception& e) {
        cerr << "Error at index " << index << ": " << e.what() << endl;
        break;
      }
      index += 128;

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