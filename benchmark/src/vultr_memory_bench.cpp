#include <benchmark/benchmark.h>
#include <core/memory/vultr_memory.h>
#include <core/memory/pool.h>
#include <core/memory/free_list.h>

static void bm_malloc(benchmark::State &state)
{
    srand(1);
    for (auto _ : state)
    {
        void *data = malloc(rand() % Kilobyte(1));
    }
}
BENCHMARK(bm_malloc)->Iterations(90000);

static void bm_pool_alloc_single(benchmark::State &state)
{
    using namespace Vultr;
    srand(1);
    MemoryArena *arena       = init_mem_arena(Gigabyte(4));
    PoolAllocator *allocator = init_pool_allocator(arena, Kilobyte(1), 9000);

    for (auto _ : state)
    {
        pool_alloc(allocator, rand() % Kilobyte(1));
    }
    destroy_mem_arena(arena);
}
BENCHMARK(bm_pool_alloc_single)->Iterations(90000);

static void bm_pool_alloc_multi(benchmark::State &state)
{
    using namespace Vultr;
    srand(1);
    MemoryArena *arena       = init_mem_arena(Gigabyte(4));
    PoolRegion regions[]     = {{.size = 128, .count = 40000}, {.size = 256, .count = 30000}, {.size = 512, .count = 10000}, {.size = Kilobyte(1), .count = 10000}};
    PoolAllocator *allocator = init_pool_allocator(arena, Kilobyte(1), 9000);

    for (auto _ : state)
    {
        pool_alloc(allocator, rand() % Kilobyte(1));
    }
    destroy_mem_arena(arena);
}
BENCHMARK(bm_pool_alloc_multi)->Iterations(90000);

static void bm_free_list_alloc(benchmark::State &state)
{
    using namespace Vultr;
    srand(1);
    MemoryArena *arena           = init_mem_arena(Gigabyte(4));
    FreeListAllocator *allocator = init_free_list_allocator(arena, Gigabyte(3) + Megabyte(512), 16);
    for (auto _ : state)
    {
        void *data = free_list_alloc(allocator, rand() % Kilobyte(1));
    }
    destroy_mem_arena(arena);
}
BENCHMARK(bm_free_list_alloc)->Iterations(90000);
BENCHMARK_MAIN();
