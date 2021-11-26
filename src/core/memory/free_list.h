// TODO(Brandon): Update documentation.
#pragma once
#include <types/types.h>
#include "vultr_memory.h"

namespace Vultr
{
    struct FreeListAllocator;

    FreeListAllocator *init_free_list_allocator(MemoryArena *arena, size_t size, u8 alignment);

    /*
     * Allocate a chunk of memory from a `MemoryArena`.
     * @param MemoryArena *arena: The memory arena to allocate from.
     * @param size_t size: The size of memory to allocate.
     *
     * @return void *: The memory that can now be used.
     *
     * @error The method will return nullptr if there is no memory chunk available to allocate.
     *
     * @no_thread_safety
     * */
    void *free_list_alloc(FreeListAllocator *allocator, size_t size);

    /*
     * Resize a chunk of memory from a `MemoryArena`.
     * @param MemoryArena *arena: The memory arena to allocate from.
     * @param void *: The old allocated block of memory.
     * @param size_t size: The size of memory to allocate.
     *
     * @return void *: The memory that can now be used.
     *
     * @error The method will return nullptr if there is no memory chunk available to allocate.
     *
     * @no_thread_safety
     * */
    void *free_list_realloc(FreeListAllocator *allocator, void *data, size_t size);

    /*
     * Free a chunk of memory from a `MemoryArena`.
     * @param MemoryArena *mem: The memory arena to free from.
     * @param void *data: The data to free.
     *
     * @no_thread_safety
     * */
    void free_list_free(FreeListAllocator *allocator, void *data);

} // namespace Vultr
