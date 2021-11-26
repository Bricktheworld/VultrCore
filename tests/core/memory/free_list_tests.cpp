#include <gtest/gtest.h>
#define private public
#define protected public

#include <core/memory/vultr_memory.h>
#include <core/memory/free_list.h>
#include <stdlib.h>
#include <math.h>

using namespace Vultr;
TEST(FreeListTests, AllocFree)
{
    MemoryArena *arena = init_mem_arena(Kilobyte(512));
    auto *allocator    = init_free_list_allocator(arena, Kilobyte(256), 16);
    ASSERT_NE(allocator, nullptr);
    const u32 count = 10;

    size_t starting_size = Kilobyte(512) - 24;

    void *allocations[count];
    size_t allocation_sizes[count];
    for (u32 i = 0; i < count; i++)
    {
        size_t size         = (i + 32) << 3;
        allocation_sizes[i] = size;
        allocations[i]      = free_list_alloc(allocator, size);
        ASSERT_NE(allocations[i], nullptr);
    }

    for (s32 i = count - 1; i >= 0; i--)
    {
        free_list_free(allocator, allocations[i]);
    }

    destroy_mem_arena(arena);
}

TEST(FreeListTests, AllocRealloc)
{
    // TODO(Brandon): Make more robust tests if there are new edge cases found.
    MemoryArena *arena = init_mem_arena(Kilobyte(512));
    auto *allocator    = init_free_list_allocator(arena, Kilobyte(256), 16);
    ASSERT_NE(allocator, nullptr);

    {
        void *data = free_list_alloc(allocator, Kilobyte(1));
        ASSERT_NE(data, nullptr);
        void *new_data = free_list_realloc(allocator, data, Kilobyte(2));
        ASSERT_EQ(new_data, data);
        free_list_free(allocator, new_data);
    }

    {
        void *data = free_list_alloc(allocator, Kilobyte(1));
        ASSERT_NE(data, nullptr);

        void *second_data = free_list_alloc(allocator, Kilobyte(2));
        ASSERT_NE(second_data, nullptr);

        void *new_data = free_list_realloc(allocator, data, Kilobyte(3));
        ASSERT_NE(new_data, nullptr);
        ASSERT_NE(new_data, data);
    }

    destroy_mem_arena(arena);
}
