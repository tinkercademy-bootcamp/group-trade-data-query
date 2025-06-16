#include "segment_tree.h"
#include <cmath>

void merge(seg_node &curr_node, const seg_node &left_node, const seg_node &right_node){
  curr_node.lowest_price = left_node.lowest_price.price*std::pow(10,left_node.lowest_price.price_exponent) < right_node.lowest_price.price*std::pow(10,right_node.lowest_price.price_exponent) ? left_node.lowest_price : right_node.lowest_price;
  curr_node.highest_price = left_node.highest_price.price*std::pow(10,left_node.highest_price.price_exponent) > right_node.highest_price.price*std::pow(10,right_node.highest_price.price_exponent) ? left_node.highest_price : right_node.highest_price;
}

void merge(seg_node &curr_node, const seg_node &other_node){
  curr_node.lowest_price = curr_node.lowest_price.price*std::pow(10,curr_node.lowest_price.price_exponent) < other_node.lowest_price.price*std::pow(10,other_node.lowest_price.price_exponent) ? curr_node.lowest_price : other_node.lowest_price;
  curr_node.highest_price = curr_node.highest_price.price*std::pow(10,curr_node.highest_price.price_exponent) > other_node.highest_price.price*std::pow(10,other_node.highest_price.price_exponent) ? curr_node.highest_price : other_node.highest_price;
}

SegtreeBin::SegtreeBin() {
  data.open("data/processed/segment-tree.bin", std::ios::in | std::ios::binary);
  if (!data.is_open()) {
    std::cerr << "[Segment Tree] Error: could not open segment tree binary file.\n";
    return;
  }
  n = (data.seekg(0, std::ios::end).tellg() / sizeof(seg_node)) >> 1;
  data.seekg(0, std::ios::beg);
}

bool SegtreeBin::read_segtree_data(uint64_t ind, seg_node& sn) {
  data.seekg(ind * sizeof(seg_node), std::ios::beg);
  if (data.read(reinterpret_cast<char *>(&sn), sizeof(seg_node))) {
    return true;
  } else {
    throw std::runtime_error("[Segment Tree] Error reading segment tree data at index " + std::to_string(ind));
  }
  return false; // If we reach here, it means reading failed
}

seg_node SegtreeBin::bin_query(int64_t l, int64_t r){
  seg_node res = {{UINT32_MAX, 0}, {0, 0}};
  seg_node sn;

  for (l += n, r += n; l <= r; l >>= 1, r >>= 1) {
    if(l & 1){
      read_segtree_data(l++, sn);
      merge(res, sn);        
    }
    if(!(r & 1)){
      read_segtree_data(r--, sn);
      merge(res, sn);
    }
  }

  return res;
}
