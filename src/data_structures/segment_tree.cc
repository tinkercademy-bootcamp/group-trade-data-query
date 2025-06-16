#include "../query_engine/query_engine.h"
#include "../utils/query.h"

struct seg_node{
    Price lowest_price;
    Price highest_price;
}

struct Segtree{
    std::vector<seg_node> tr;
    uint64_t n;
    Query_engine qe;

    void init(){
        n = siz;
        tr.resize(2 * n);
                
    }
};