#include <gtest/gtest.h>
#define private public
#define protected public

#include <core/memory/vultr_memory.h>
#include <core/memory/linear.h>

using namespace Vultr;
TEST(LinearTests, AllocFree)
{
    MemoryArena *arena = init_mem_arena(Kilobyte(512));
    auto *allocator    = init_linear_allocator(arena, Kilobyte(256));

    ASSERT_NE(allocator, nullptr);

    const u32 count = 10;

    ASSERT_EQ(linear_alloc(allocator, Megabyte(1)), nullptr);

    void *allocations[count];
    size_t allocation_sizes[count];

    for (u32 i = 0; i < count; i++)
    {
        size_t size         = (i + 32) << 3;
        allocation_sizes[i] = size;
        allocations[i]      = linear_alloc(allocator, size);
        ASSERT_NE(allocations[i], nullptr);
    }

    linear_free(allocator);

    destroy_mem_arena(arena);
}
