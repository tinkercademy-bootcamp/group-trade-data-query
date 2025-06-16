#ifndef SEGMENT_TREE_H
#define SEGMENT_TREE_H

#include "../query_engine/query_engine.h"
#include "../utils/query.h"

struct seg_node{
  Price lowest_price;
  Price highest_price;
};

void merge(seg_node &curr_node, const seg_node &left_node, const seg_node &right_node);
void merge(seg_node &curr_node, const seg_node &other_node);

struct Segtree{
  std::vector<seg_node> segtree_arr;
  uint64_t n;
  Query_engine qe;

  Segtree();
  seg_node query(int64_t l, int64_t r);
};

#endif