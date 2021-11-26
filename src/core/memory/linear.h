#pragma once
#include <types/types.h>
#include "vultr_memory.h"

namespace Vultr
{
    struct LinearAllocator;

    LinearAllocator *init_linear_allocator(MemoryArena *arena, size_t size);

    void *linear_alloc(LinearAllocator *allocator, size_t size);

    void linear_free(LinearAllocator *allocator);
} // namespace Vultr
