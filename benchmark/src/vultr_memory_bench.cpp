#include <benchmark/benchmark.h>
#include <core/memory/memory.h>
#include <core/memory/pool.h>
#include <core/memory/free_list.h>
#include <core/memory/slab.h>

static void bm_malloc(benchmark::State &state)
{
	u32 counter = 0;
	void *allocated[9000];
	srand(1);
	for (auto _ : state)
	{
		allocated[counter] = malloc(rand() % Kilobyte(1));
		counter++;
		if (counter == 9000)
		{
			for (u32 i = 0; i < 9000; i++)
			{
				free(allocated[i]);
			}
			counter = 0;
		}
	}
}
BENCHMARK(bm_malloc);

static void bm_free_list_alloc(benchmark::State &state)
{
	using namespace Vultr;
	srand(1);
	MemoryArena *arena           = init_mem_arena(Gigabyte(4));
	FreeListAllocator *allocator = init_free_list_allocator(arena, Gigabyte(3) + Megabyte(512), 16);

	u32 counter                  = 0;
	void *allocated[9000];

	for (auto _ : state)
	{
		allocated[counter] = free_list_alloc(allocator, rand() % Kilobyte(1));
		counter++;
		if (counter == 9000)
		{
			for (auto &i : allocated)
			{
				free_list_free(allocator, i);
			}
			counter = 0;
		}
	}
	destroy_mem_arena(arena);
}
BENCHMARK(bm_free_list_alloc);

static void bm_slab_alloc(benchmark::State &state)
{
	using namespace Vultr;
	srand(1);
	auto *arena                    = init_mem_arena(Gigabyte(4));
	SlabDeclaration declarations[] = {{.block_size = 64, .count = 128},  {.block_size = 128, .count = 128},         {.block_size = 256, .count = 128},
									  {.block_size = 512, .count = 128}, {.block_size = Kilobyte(1), .count = 128}, {.block_size = Kilobyte(2), .count = 128}};
	auto *slab_allocator           = init_slab_allocator(arena, declarations, 6);

	u32 counter                    = 0;
	void *allocated[9000];

	srand(1);
	for (auto _ : state)
	{
		allocated[counter] = slab_alloc(slab_allocator, rand() % Kilobyte(1));

		counter++;
		if (counter == 9000)
		{
			for (u32 i = 0; i < 9000; i++)
			{
				slab_free(slab_allocator, allocated[i]);
			}
			counter = 0;
		}
	}
	destroy_mem_arena(arena);
}
BENCHMARK(bm_slab_alloc);

static void bm_pool_alloc(benchmark::State &state)
{
	using namespace Vultr;
	srand(1);
	MemoryArena *arena       = init_mem_arena(Gigabyte(4));
	PoolRegion regions[]     = {{.size = 128, .count = 40000}, {.size = 256, .count = 30000}, {.size = 512, .count = 10000}, {.size = Kilobyte(1), .count = 10000}};
	PoolAllocator *allocator = init_pool_allocator(arena, Kilobyte(1), 9000);

	u32 counter              = 0;
	void *allocated[9000];

	for (auto _ : state)
	{
		allocated[counter] = pool_alloc(allocator, rand() % Kilobyte(1));
		counter++;
		if (counter == 9000)
		{
			for (auto &i : allocated)
			{
				pool_free(allocator, i);
			}
			counter = 0;
		}
	}
	destroy_mem_arena(arena);
}
BENCHMARK(bm_pool_alloc);

BENCHMARK_MAIN();
