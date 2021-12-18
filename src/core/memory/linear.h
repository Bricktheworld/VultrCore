#pragma once
#include <types/types.h>
#include "vultr_memory_internal.h"

namespace Vultr
{
    /**
     * Allocator which allocates all memory blocks in O(1) time however memory can only be freed by freeing all allocated memory.
     */
    struct LinearAllocator : public Allocator
    {
        size_t size = 0;
        size_t used = 0;
        bool locked = false;
        void *next = nullptr;

        LinearAllocator() : Allocator(AllocatorType::Linear) {}
    };

    /**
     * Initialize a new linear allocator. This allocator is best used for memory that is allocated at the start of the program and isn't freed until the end of the program or temporary memory that is allocated once
     * then freed immediately.
     * This allocator has no memory fragmentation and is very fast but you cannot free individual elements and must instead free all allocated memory.
     *
     * @param MemoryArena *arena: The memory arena to create the allocator from.
     * @param size_t size: The total size that this allocator will be able to allocate in bytes.
     *
     * @return LinearAllocator *: The allocator which can be now be used.
     *
     * @error The method will return nullptr if the memory arena has run out of memory and cannot allocate.
     *
     * @no_thread_safety
     */
    LinearAllocator *init_linear_allocator(MemoryArena *arena, size_t size);

    /**
     * Allocate a chunk of memory from using a linear allocator.
     *
     * @param LinearAllocator *allocator: The allocator to use.
     * @param size_t size: The size of memory to allocate.
     *
     * @return void *: The memory that can now be used.
     *
     * @error The method will return nullptr if the linear allocator doesn't have enough space to allocate.
     *
     * @no_thread_safety
     */
    void *linear_alloc(LinearAllocator *allocator, size_t size);

    /**
     * Free all allocated memory from a linear allocator.
     * @param LinearAllocator *allocator: The allocator to use.
     *
     * @no_thread_safety
     */
    void linear_free(LinearAllocator *allocator);
} // namespace Vultr
