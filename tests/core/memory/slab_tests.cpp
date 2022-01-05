#include <gtest/gtest.h>
#define private public
#define protected public
#include <core/memory/slab.h>

using namespace Vultr;
TEST(SlabTests, All)
{
	MemoryArena *arena             = init_mem_arena(Gigabyte(1));
	SlabDeclaration declarations[] = {{.block_size = 64, .count = 128},  {.block_size = 128, .count = 128},         {.block_size = 256, .count = 128},
									  {.block_size = 512, .count = 128}, {.block_size = Kilobyte(1), .count = 128}, {.block_size = Kilobyte(2), .count = 128}};
	auto *allocator                = init_slab_allocator(arena, declarations, 6);
	ASSERT_NE(allocator, nullptr);

	void *data = slab_alloc(allocator, 10);
	ASSERT_NE(data, nullptr);

	void *second_data = slab_alloc(allocator, 10);
	ASSERT_NE(second_data, nullptr);
	ASSERT_EQ((reinterpret_cast<byte *>(second_data) - reinterpret_cast<byte *>(data)), 64);

	slab_free(allocator, data);
	void *third_data = slab_alloc(allocator, 64);
	ASSERT_EQ(third_data, data);

	data = slab_realloc(allocator, third_data, 65);
	ASSERT_NE(data, nullptr);
	ASSERT_NE(data, third_data);
	void *reallocated_data = slab_realloc(allocator, data, 66);
	ASSERT_EQ(reallocated_data, data);
	void *second_reallocated_data = slab_realloc(allocator, data, 65);
	ASSERT_EQ(second_reallocated_data, reallocated_data);

	void *fourth_data = slab_alloc(allocator, 10);
	ASSERT_EQ(fourth_data, third_data);

	void *third_reallocated_data = slab_realloc(allocator, data, 64);
	ASSERT_NE(third_reallocated_data, second_reallocated_data);
}