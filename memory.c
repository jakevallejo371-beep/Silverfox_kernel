#include "memory.h"
#include "vga.h"
#include "kstr.h"

#include <stddef.h>
#include <stdint.h>

memory_t g_memory;

void printRAMib(uint32_t kib)
{
    if (kib >= 1024U * 1024U)
    {
        print(int_to_str(kib / (1024U * 1024U)));
        print(" MiB");
    }
    else if (kib >= 1024U)
    {
        print(int_to_str(kib / 1024U));
        print(" KiB");
    }
    else
    {
        print(int_to_str(kib));
        print(" KiB");
    }
}

void printRAMb(uint32_t kib)
{
    uint32_t bytes = kib * 1024U;

    if (bytes >= 1000000U)
    {
        print(int_to_str(bytes / 1000000U));
        print(" MB");
    }
    else if (bytes >= 1000U)
    {
        print(int_to_str(bytes / 1000U));
        print(" kB");
    }
    else
    {
        print(int_to_str(bytes));
        print(" bytes");
    }
}