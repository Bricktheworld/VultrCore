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
}