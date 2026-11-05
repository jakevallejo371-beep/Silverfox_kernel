#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

#include <stdint.h>


typedef struct {
    uint8_t *buffer;
    size_t capacity;
    size_t offset;
} bump_allocator;

void bump_init(bump_allocator* arena, void* backing_buffer, size_t cap);
int install_bump(void);

extern bump_allocator arena;

void* bump(bump_allocator* arena, size_t bytes);
void bump_reset(bump_allocator* arena);
void OOM_handler(void);

#endif // ALLOCATOR