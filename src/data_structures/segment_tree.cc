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

Segtree::Segtree(){
    n = qe.trades_size;
    segtree_arr.resize(2 * n);
    TradeData trade;

    for(uint64_t i = 0; i < n; ++i){
        qe.read_trade_data(i,trade);
        segtree_arr[i+n] = {trade.price, trade.price};
    }

    for(uint64_t i = n-1; i >= 1; --i){
        merge(segtree_arr[i], segtree_arr[i<<1], segtree_arr[i<<1|1]);
    }
  }

seg_node Segtree::query(int64_t l, int64_t r){
  seg_node res = {{UINT32_MAX, 0}, {0, 0}};

  for (l += n, r += n; l <= r; l >>= 1, r >>= 1) {
    if(l & 1){
      merge(res, segtree_arr[l++]);        
    }
    if(!(r & 1)){
      merge(res, segtree_arr[r--]);
    }
  }

  return res;
};