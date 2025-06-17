#ifndef SEGMENT_TREE_H
#define SEGMENT_TREE_H

#include "../utils/query.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <mutex>

struct seg_node{
  Price lowest_price;
  Price highest_price;
};

void merge(seg_node &curr_node, const seg_node &left_node, const seg_node &right_node);
void merge(seg_node &curr_node, const seg_node &other_node);

struct SegtreeBin{
  uint64_t n;
  std::mutex mtx;

  SegtreeBin();
  bool read_segtree_data(uint64_t ind, seg_node& sn);
  seg_node bin_query(int64_t l, int64_t r);

  private:
    std::ifstream data;
};

#endif