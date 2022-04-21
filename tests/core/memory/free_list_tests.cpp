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
		ASSERT_NE(new_data, data);
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

TEST(FreeListTests, RBTBug)
{
	MemoryArena *arena = init_mem_arena(Gigabyte(1));
	auto *allocator    = init_free_list_allocator(arena, Megabyte(900), 16);

	free_list_alloc(allocator, 8000);
	free_list_alloc(allocator, 40000);
	free_list_alloc(allocator, 8000);
	free_list_alloc(allocator, 64000);
	free_list_alloc(allocator, 40000);
	free_list_alloc(allocator, 4240);
	auto *p3 = free_list_alloc(allocator, 753040);
	auto *p1 = free_list_alloc(allocator, 4992);
	auto *p2 = free_list_alloc(allocator, 10704);
	free_list_alloc(allocator, 52544);
	auto *p4 = free_list_alloc(allocator, 119104);
	free_list_free(allocator, p2);
	free_list_free(allocator, p1);
	free_list_alloc(allocator, 753040);
	free_list_alloc(allocator, 119104);

	free_list_free(allocator, p4);
	free_list_free(allocator, p3);

	auto *p5 = free_list_alloc(allocator, 452592);
	auto *p6 = free_list_alloc(allocator, 83280);
	//	auto *p7 = free_list_alloc(allocator, 452592);
	//	auto *p8 = free_list_alloc(allocator, 83280);
	//	free_list_free(allocator, p6);
	//	free_list_free(allocator, p5);
	//	free_list_alloc(allocator, 254352);
	//	free_list_alloc(allocator, 37920);
	//	auto *p9  = free_list_alloc(allocator, 254352);
	//	auto *p10 = free_list_alloc(allocator, 37920);
	//	free_list_free(allocator, p10);
	//	free_list_free(allocator, p9);
	//
	//	free_list_free(allocator, p7);
	//	free_list_free(allocator, p8);
	//
	//	free_list_alloc(allocator, 254352);
	//	free_list_alloc(allocator, 37920);
	//	free_list_alloc(allocator, 254352);
	//	free_list_print(allocator);
	destroy_mem_arena(arena);
}
