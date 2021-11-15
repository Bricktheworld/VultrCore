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

void print(MemoryArena *arena, MemoryBlock *n, void (*print_func)(size_t size), int depth, const char *label)
{
    if (n != nullptr)
    {
        print(arena, n->free.right, print_func, depth + 1, "R");
        printf("%*s", 8 * depth, "");
        if (label)
            printf("%s: ", label);
        print_func(n->size & ~0x7);
        printf(" (%s)\n", BIT_IS_HIGH(n->size, 2) ? "r" : "b");
        print(arena, n->free.left, print_func, depth + 1, "L");
    }
}

void rbt_print(MemoryArena *arena, void (*print_func)(size_t size))
{
    printf("\n--\n");
    print(arena, arena->free_root, print_func, 0, "T");
}

TEST(MemoryArena, Allocate)
{
    MemoryArena *arena = init_mem_arena(Kilobyte(512));
    const u32 count    = 10;

    size_t starting_size = Kilobyte(512) - 24;

    void *allocations[count];
    size_t allocation_sizes[count];
    for (u32 i = 0; i < count; i++)
    {
        size_t size         = (i + 32) << 3;
        allocation_sizes[i] = size;
        allocations[i]      = mem_arena_alloc(arena, size);
    }

    for (s32 i = count - 1; i >= 0; i--)
    {
        mem_arena_free(arena, allocations[i]);
    }

    destroy_mem_arena(arena);
}
