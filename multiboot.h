#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stddef.h>

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
} multi_info_t;

#endif