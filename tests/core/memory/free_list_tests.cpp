#include <gtest/gtest.h>
#define private public
#define protected public

#include <core/memory/memory.h>
#include <core/memory/free_list.h>

using namespace Vultr;
TEST(FreeListTests, AllocFree)
{
	MemoryArena *arena = init_mem_arena(Kilobyte(512));
	auto *allocator    = init_free_list_allocator(arena, Kilobyte(256), 16);
	ASSERT_NE(allocator, nullptr);

	{
		const u32 count      = 10;

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
	}
	{
		auto *broken_alloc = free_list_alloc(allocator, 16);
		free_list_free(allocator, broken_alloc);
	}

	destroy_mem_arena(arena);
}