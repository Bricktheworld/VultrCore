#include "vultr_memory_internal.h"
#include "linear.cpp"
#include "pool.cpp"
#include "free_list.cpp"

namespace Vultr
{
    MemoryArena *init_mem_arena(size_t size, u8 alignment)
    {
        // Virtual alloc some memory.
        auto *memory_block = Platform::virtual_alloc(nullptr, sizeof(MemoryArena) + size);

        // If it returned nullptr, then we can assume that the allocation failed.
        if (memory_block == nullptr)
        {
            return nullptr;
        }

        // The memory allocator will be at the start of this platform memory block.
        auto *arena            = reinterpret_cast<MemoryArena *>(Platform::get_memory(memory_block));
        arena->memory          = memory_block;
        arena->next_free_chunk = reinterpret_cast<byte *>(arena) + sizeof(MemoryArena);
        arena->next_index      = 0;

        return arena;
    }

    void *mem_arena_designate(MemoryArena *arena, AllocatorType type, size_t size)
    {
        ASSERT(type != AllocatorType::None, "Cannot designate an invalid allocator!");

        if (arena->next_index >= MAX_ALLOCATORS)
            return nullptr;

        if (arena->next_free_chunk == nullptr)
            return nullptr;

        void *chunk           = arena->next_free_chunk;
        size_t remaining_size = Platform::get_memory_size(arena->memory) - (reinterpret_cast<byte *>(chunk) - reinterpret_cast<byte *>(arena));
        if (remaining_size >= size)
        {
            auto index                    = arena->next_index;
            arena->allocators[index]      = chunk;
            arena->allocator_types[index] = type;
            arena->next_index++;
            return chunk;
        }
        else
        {
            return nullptr;
        }
    }

    void destroy_mem_arena(MemoryArena *arena)
    {
        ASSERT(arena != nullptr && arena->memory != nullptr, "Invalid memory arena!");
        Platform::virtual_free(arena->memory);
    }

} // namespace Vultr
