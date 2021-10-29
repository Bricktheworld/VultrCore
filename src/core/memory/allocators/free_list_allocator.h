#pragma once

namespace Vultr
{
    struct MemoryBlock
    {
    };

    struct FreeListAllocator
    {
        MemoryBlock *head = nullptr;

        FreeListAllocator() = default;

        FreeListAllocator(const FreeListAllocator &other) = delete;
        FreeListAllocator &operator=(const FreeListAllocator &other) = delete;
    };

    void init_allocator(FreeListAllocator *allocator);

    template <typename T>
    T *mem_alloc(FreeListAllocator *allocator)
    {
    }

    template <typename T>
    void mem_free(FreeListAllocator *allocator)
    {
    }
} // namespace Vultr
