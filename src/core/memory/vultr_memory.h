#pragma once
#include <types/types.h>
#include <platform/platform.h>
#include <string.h>

namespace Vultr
{
    enum struct AllocatorType : u8
    {
        None     = 0x0,
        Linear   = 0x1,
        Stack    = 0x2,
        Pool     = 0x4,
        FreeList = 0x8,
    };

    template <typename T>
    AllocatorType get_allocator_type();

#define MAX_ALLOCATORS 16
    struct MemoryArena
    {
        Platform::PlatformMemoryBlock *memory = nullptr;
        AllocatorType allocator_types[MAX_ALLOCATORS];
        void *allocators[MAX_ALLOCATORS];
        u32 next_index        = 0;
        void *next_free_chunk = nullptr;
    };

    /**
     * Allocate a chunk of memory from the OS and put it in a `MemoryArena`.
     * @param size_t size: Size in bytes of how much space the memory arena should have.
     *
     * @return MemoryArena *: The memory arena object.
     *
     * @error The method will return nullptr if it is unable to allocate the memory arena. This should be handled properly.
     */
    MemoryArena *init_mem_arena(size_t size, u8 alignment = 16);

    void *mem_arena_designate(MemoryArena *arena, AllocatorType type, size_t size);

    void *mem_arena_get_allocator(MemoryArena *arena, void *data);

    /**
     * Free a `MemoryArena` from the OS.
     * @param MemoryArena *mem: The memory arena to be freed.
     *
     */
    void destroy_mem_arena(MemoryArena *arena);
} // namespace Vultr
