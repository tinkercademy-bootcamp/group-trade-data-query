#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include "../src/utils/query.h"
#include "../src/data_structures/segment_tree.h"
#include "../src/query_engine/query_engine.h"

void build_segtree(std::ofstream& out){
  Query_engine qe;
  uint64_t n = qe.trades_size;
  std::vector<seg_node> segtree_arr(2*n);
  
  TradeData trade;

  for(uint64_t i = 0; i < n; ++i){
      qe.read_trade_data(i,trade);
      segtree_arr[i+n] = {trade.price, trade.price};
  }

  for(uint64_t i = n-1; i >= 1; --i){
      merge(segtree_arr[i], segtree_arr[i<<1], segtree_arr[i<<1|1]);
  }
  
  out.write(reinterpret_cast<const char *>(segtree_arr.data()), segtree_arr.size() * sizeof(seg_node));
}

// Simple test to verify parsing
int32_t main(int32_t argc, char** argv) {
  std::string out_path = "../data/processed/segment-tree.bin";
  std::ofstream out(out_path, std::ios::out | std::ios::trunc | std::ios::binary);

  if (!out.is_open()) {
    std::cerr << "Error: could not open output file\n";
    return 1;
  }

  build_segtree(out);
  
  return 0;
}