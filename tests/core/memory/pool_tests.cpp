#include <gtest/gtest.h>
#define private public
#define protected public

#include <core/memory/memory.h>
#include <core/memory/pool.h>

using namespace Vultr;

TEST(PoolTests, AllocFree)
{
	MemoryArena *arena = init_mem_arena(Kilobyte(512));
	auto *allocator    = init_pool_allocator(arena, 128, 100);

	ASSERT_NE(allocator, nullptr);
	ASSERT_EQ(pool_alloc(allocator, 256), nullptr);

	{
		void *data = pool_alloc(allocator, 128);
		ASSERT_NE(data, nullptr);
		pool_free(allocator, data);
	}

	{
		void *data = pool_alloc(allocator, 128);
		ASSERT_NE(data, nullptr);

		void *second_data = pool_alloc(allocator, 100);
		ASSERT_NE(second_data, nullptr);
		ASSERT_NE(second_data, data);

		pool_free(allocator, data);

		void *third_data = pool_alloc(allocator, 128);
		ASSERT_NE(third_data, nullptr);
		ASSERT_EQ(third_data, data);
	}

	destroy_mem_arena(arena);
}
