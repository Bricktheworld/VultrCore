#include "vultr_memory.h"

namespace Vultr
{
    MemoryArena *alloc_mem_arena(u64 size, u8 alignment)
    {
        // NOTE(Brandon): These should really be the only two places where malloc and free are ever called throughout the lifetime of the program.
        // Every other dynamic allocation should be done through the memory arenas.

        // TODO(Brandon): Replace malloc with a more performant, platform specific, method
        auto *mem = static_cast<MemoryArena *>(malloc(sizeof(MemoryArena)));

        if (mem == nullptr)
        {
            return nullptr;
        }
        mem->_memory_chunk = malloc(size);
        mem->alignment = alignment;

        MemoryBlock head;
        head.allocated = false;
        head.data = mem->_memory_chunk;

        // Subtract the size of the memory header because this will exist at all times
        head.size = size - sizeof(MemoryHeader);

        mem->head = head;

        return mem;
    }

    void *mem_arena_alloc(MemoryArena *arena, u64 size)
    {
    }

    void mem_arena_free(MemoryArena *arena, void *data)
    {
    }

    void mem_arena_free(MemoryArena *mem)
    {
        free(mem);
    }
} // namespace Vultr
