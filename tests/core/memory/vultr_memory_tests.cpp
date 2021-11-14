#include <gtest/gtest.h>
#define private public
#define protected public

#include <core/memory/vultr_memory.h>
#include <stdlib.h>
#include <math.h>

using namespace Vultr;
TEST(MemoryArena, InitMemArena)
{
    MemoryArena *arena = init_mem_arena(Megabyte(256));
    ASSERT_NE(arena, nullptr);
    destroy_mem_arena(arena);

    arena = init_mem_arena(Terabyte(10));
    ASSERT_EQ(arena, nullptr);
}
static void init_free_mb(MemoryBlock *block, u64 size, MemoryBlock *prev, MemoryBlock *next)
{
    block->size        = (~(1UL << 1) & size) | (1UL << 0);
    block->next        = next;
    block->prev        = prev;
    block->free.parent = nullptr;
    block->free.center = nullptr;
    block->free.left   = nullptr;
    block->free.right  = nullptr;
}

TEST(MemoryArena, Allocate)
{
    MemoryArena *arena = init_mem_arena(2048);
    const u32 count    = 10;

    auto **blocks = static_cast<MemoryBlock **>(malloc(sizeof(MemoryBlock *) * count));
    for (u32 i = 0; i < count; i++)
    {
        auto *b   = static_cast<MemoryBlock *>(malloc(sizeof(MemoryBlock)));
        blocks[i] = b;
        // init_free_mb(b, rand() % 2048, nullptr, nullptr);
        init_free_mb(b, i << 3, nullptr, nullptr);
        rbt_print(arena, [](u64 size) { printf("%lu", size); });
        rbt_insert(arena, b);
    }
    rbt_print(arena, [](u64 size) { printf("%lu", size); });

    for (s32 i = count - 1; i >= 0; i--)
    {
        auto *b = blocks[i];
        rbt_delete(arena, b);
        rbt_print(arena, [](u64 size) { printf("%lu", size); });
        free(b);
    }
    free(blocks);

    // void *buf          = mem_arena_alloc(arena, 1024);
    // mem_arena_free(arena, buf);
    destroy_mem_arena(arena);
}
