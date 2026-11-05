#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

typedef struct {
    uint32_t kib;
    uint32_t kb;
    uint32_t mb;
} memory_t;

extern memory_t g_memory;

void printRAMib(uint32_t kib);
void printRAMb(uint32_t kib);

typedef struct {

} alloc_block_t;

#endif