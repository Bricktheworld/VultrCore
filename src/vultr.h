#pragma once

#include "core/memory/vultr_memory.h"
namespace Vultr
{
#define THREAD_SAFE_ARENAS 8
    // ts = thread_safe
    struct GameMemory
    {
        // Not thread safe, use wisely...
        MemoryArena *main_heap = nullptr;

        // Thread safe arenas.
        MemoryArena *ts_heaps[THREAD_SAFE_ARENAS];
    };

    /**
     * Best match find memory arena. If threadsafe, this will try to find a memory arena that isn't currently locked
     * If not threadsafe, then just get the main heap and use that.
     * @param size_t size: Size in bytes of how much space the memory arena should have.
     *
     * @return MemoryArena *: Pointer to the heap to use.
     */
    MemoryArena *get_memory_arena(bool threadsafe);

    /**
     * Heap allocate a size of memory.
     * @param size_t size: Size in bytes of how much space the memory arena should have.
     *
     * @return T *: Pointer to the newly allocated memory.
     *
     * @no_thread_safety
     */
    template <typename T>
    T *malloc(size_t size)
    {
    }

    /**
     * Heap allocate a size of memory with thread safety. This will lock a memory arena and if there are not many heaps then this can cause locking issues.
     * @param size_t size: Size in bytes of how much space the memory arena should have.
     *
     * @return T *: Pointer to the newly allocated memory.
     *
     * @thread_safe
     */
    template <typename T>
    T *ts_malloc(size_t size)
    {
        // TODO(Brandon): Implement this.
        NOT_IMPLEMENTED("Need to implement threads first.");
    }

    // TODO(Brandon): Figure out how to tell which memory arena a pointer belongs to.
    /**
     * Free heap allocated memory.
     * @param size_t size: Size in bytes of how much space the memory arena should have.
     *
     * @return T *: Pointer to the newly allocated memory.
     *
     * @thread_safe
     */
    void free(void *buf);
} // namespace Vultr
