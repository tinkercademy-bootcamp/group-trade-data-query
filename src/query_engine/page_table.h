#pragma once
#include <cstdint>

#define PAGE_SIZE 4*1024*1024

struct pte_t {
    uint64_t created_at;
    uint64_t index;
};

struct pgtable_t {
    pte_t entries[PAGE_SIZE / sizeof(pte_t)];
};
