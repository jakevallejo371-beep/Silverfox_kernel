#include <stddef.h>
#include <stdint.h>

#include "allocator.h"
#include "asm.h"
#include "vga.h"


void bump_init(bump_allocator* arena, void* backing_buffer, size_t cap) {
    arena->buffer = (uint8_t*)backing_buffer;
    arena->capacity = cap;
    arena->offset = 0;
}

static uint8_t memory[4096];
bump_allocator arena;

int install_bump(void) {
    bump_init(&arena, memory, sizeof(memory));
    return 1;
}

void* bump(bump_allocator* arena, size_t bytes) {
    if (bytes > arena->capacity - arena->offset)
    {
        trigger_interrupt(0x32);
        return NULL;
    }

    void* ptr = arena->buffer + arena->offset;
    arena->offset += bytes;
    return ptr;
}

void bump_reset(bump_allocator* arena) {
    arena->offset =0;
}

void OOM_handler(void) {
    print("Interrupt: Out of memory!\n");
}