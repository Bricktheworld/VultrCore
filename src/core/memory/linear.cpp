#include "linear.h"

namespace Vultr
{
    static void *next_start(LinearAllocator *allocator) { return allocator + 1; }

    LinearAllocator *init_linear_allocator(MemoryArena *arena, size_t size)
    {
        // Designate a region within the memory arena for our allocator.
        auto *allocator = static_cast<LinearAllocator *>(mem_arena_designate(arena, AllocatorType::Linear, size + sizeof(LinearAllocator)));

        // If we were unable to allocate the required size, then there is nothing to do.
        if (allocator == nullptr)
            return nullptr;

        // The head block will come after the memory allocator.
        allocator->next = next_start(allocator);
        allocator->size = size;
        allocator->used = 0;
        allocator->type = AllocatorType::Linear;

        return allocator;
    }

    void *linear_alloc(LinearAllocator *allocator, size_t size)
    {
        // If there isn't enough space then there is nothing to do.
        if (size > allocator->size - allocator->used)
        {
            return nullptr;
        }

        void *data      = allocator->next;
        allocator->next = reinterpret_cast<byte *>(allocator->next) + size;
        allocator->used += size;

        return data;
    }

    void linear_free(LinearAllocator *allocator) { allocator->next = next_start(allocator); }

} // namespace Vultr
