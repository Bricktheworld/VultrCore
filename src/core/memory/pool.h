#pragma once
#include <types/types.h>
#include "vultr_memory.h"

namespace Vultr
{
    struct PoolAllocator;

    struct PoolRegion
    {
        u32 size;
        u32 count;
    };

    PoolAllocator *init_pool_allocator(MemoryArena *arena, u32 allocation_size, u32 count);
    PoolAllocator *init_pool_allocator(MemoryArena *arena, PoolRegion *regions, u32 region_count);

    void *pool_alloc(PoolAllocator *allocator, size_t size);

    void *pool_realloc(PoolAllocator *allocator, void *data, size_t size);

    void pool_free(PoolAllocator *allocator, void *data);
} // namespace Vultr
