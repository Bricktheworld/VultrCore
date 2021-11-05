#pragma once
#include <types/types.h>

namespace Vultr
{
    // TODO(Brandon): Remove these.
#define RED 1
#define BLACK 0
    struct Node
    {
        u32 data = 0;
        bool color = RED;
        Node *left = nullptr;
        Node *right = nullptr;
        Node *parent = nullptr;
    };

    struct RBTree
    {
        Node *root = nullptr;
    };

    Node *rbt_insert(Node *h, Node *n);
    void rbt_insert(RBTree *tree, Node *n);
    Node *rbt_delete(Node *h, Node *n);
    void rbt_delete(RBTree *t, Node *n);
    Node *rbt_search(RBTree *t, u32 data);
    u32 rbt_height(RBTree *t);

    /*
     * Present at the beginning of every block of memory
     * */
    struct MemoryHeader
    {
        // 8 Bytes dedicated to a header which just indicates the size of the chunk.
        // Low bit stores whether it is allocated or not. 1 = Allocated, 0 = Freed.
        u64 size = 0;

        // NOTE(Brandon): The following members are only valid if the memory block is free.
        void *left_child = nullptr;
        void *right_child = nullptr;

        // Low bit stores color. 1 = red, 0 = black
        void *parent = nullptr;
    };

    /*
     * Describes a section within a memory arena's allocated memory
     * */
    struct MemoryBlock
    {
        // Size in bytes of the memory block
        u64 size = 0;

        // Pointer to actual memory in block
        void *data = nullptr;

        // Whether memory block has been allocated
        bool allocated = false;

        // Pointer to next block _header_
        void *next_block = nullptr;
    };

    struct MemoryArena
    {
        // TODO(Brandon): Add support for 32 bit alignment (8 bytes)
        u8 alignment = 16;
        MemoryBlock head;

        // Internal memory chunk which should NOT be accessed
        void *_memory_chunk = nullptr;
    };

    /*
     * Allocate a chunk of memory from the OS and put it in a `MemoryArena`.
     * @param u64 size: Size in bytes of how much space the memory arena should have.
     *
     * @return MemoryArena *: The memory arena object.
     *
     * @error The method will return nullptr if there it is unable to allocate the memory arena. This should be handled properly.
     * */
    MemoryArena *alloc_mem_arena(u64 size, u8 alignment = 16);

    /*
     * Allocate a chunk of memory from a `MemoryArena`.
     * @param MemoryArena *arena: The memory arena to allocate from.
     * @param u64 size: The size of memory to allocate.
     *
     * @return void *: The memory that can now be used.
     *
     * @error The method will return nullptr if there is no memory chunk available to allocate.
     *
     * @no_thread_safety
     * */
    void *mem_arena_alloc(MemoryArena *arena, u64 size);

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
    void mem_arena_free(MemoryArena *mem);

} // namespace Vultr
