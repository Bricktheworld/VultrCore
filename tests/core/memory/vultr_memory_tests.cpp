#include <gtest/gtest.h>
#define private public
#define protected public

#include <core/memory/vultr_memory.h>

using namespace Vultr;
TEST(MemoryArena, InitMemArena)
{
    MemoryArena *arena = init_mem_arena(Megabyte(256));
    ASSERT_NE(arena, nullptr);
    destroy_mem_arena(arena);

    arena = init_mem_arena(Terabyte(10));
    ASSERT_EQ(arena, nullptr);
}
