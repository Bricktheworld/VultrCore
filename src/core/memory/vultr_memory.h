#pragma once
#include <types/types.h>

namespace Vultr
{
    struct MemoryBlock;

    // Allocated memory does not have any additional fields.
    struct AllocatedMemory
    {
    };

    struct FreeMemory
    {
        MemoryBlock *parent = nullptr;
        MemoryBlock *left   = nullptr;
        MemoryBlock *right  = nullptr;
        MemoryBlock *center = nullptr;
    };

    /*
     * Present at the beginning of every block of memory
     * Size: (64-bit) Allocated: 24 bytes, Free: 48 bytes.
     * */
    struct MemoryBlock
    {
        // Lowest 3 bits of `size` store:
        // - Whether the block has properly initialized: 0 = Uninitialized, 1 = Initialized.
        // - Whether it is allocated or not:  0 = Freed, 1 = Allocated.
        // - The color of the block (Only valid for free blocks of memory. Used for red-black tree): 0 = Black, 1 = Red
        size_t size = 0;

        // Pointer to previous and next memory blocks
        MemoryBlock *prev = nullptr;
        MemoryBlock *next = nullptr;

        union {
            AllocatedMemory allocated;
            FreeMemory free;
        };
    };

    struct MemoryArena
    {
        // TODO(Brandon): Add support for 32 bit alignment (8 bytes)
        u8 alignment           = 16;
        MemoryBlock *free_root = nullptr;
        MemoryBlock *memory    = nullptr;
    };

    /*
     * Allocate a chunk of memory from the OS and put it in a `MemoryArena`.
     * @param size_t size: Size in bytes of how much space the memory arena should have.
     *
     * @return MemoryArena *: The memory arena object.
     *
     * @error The method will return nullptr if it is unable to allocate the memory arena. This should be handled properly.
     * */
    MemoryArena *init_mem_arena(size_t size, u8 alignment = 16);

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
    void *mem_arena_alloc(MemoryArena *arena, size_t size);

    /*
     * Free a chunk of memory from a `MemoryArena`.
     * @param MemoryArena *mem: The memory arena to free from.
     * @param void *data: The data to free.
     *
     * @no_thread_safety
     * */
    void mem_arena_free(MemoryArena *arena, void *data);

    /*
     * Free a `MemoryArena` from the OS.
     * @param MemoryArena *mem: The memory arena to be freed.
     * */
    void destroy_mem_arena(MemoryArena *arena);

} // namespace Vultr
