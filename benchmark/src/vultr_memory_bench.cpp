#include <benchmark/benchmark.h>
#include <core/memory/vultr_memory.h>

static void bm_malloc(benchmark::State &state)
{
    srand(1);
    for (auto _ : state)
    {
        void *data = malloc(rand() % Kilobyte(1));
        // free(data);
    }
}
// Register the function as a benchmark
BENCHMARK(bm_malloc)->Iterations(9000);

// Define another benchmark
static void bm_arena_alloc(benchmark::State &state)
{
    using namespace Vultr;
    srand(1);
    MemoryArena *arena = init_mem_arena(Gigabyte(4));
    for (auto _ : state)
    {
        void *data = mem_arena_alloc(arena, rand() % Kilobyte(1));
        // mem_arena_free(arena, data);
    }
    destroy_mem_arena(arena);
}
BENCHMARK(bm_arena_alloc)->Iterations(9000);

static void bm_rbt_search(benchmark::State &state)
{
    using namespace Vultr;

    MemoryArena *arena = init_mem_arena(Gigabyte(4));
    for (s32 i = 0; i < 9000; i++)
    {
        auto *block = static_cast<MemoryBlock *>(malloc(sizeof(MemoryBlock)));
        init_free_mb(block, i << 3, nullptr, nullptr);
        insert_free_mb(arena, block);
    }

    srand(1);
    for (auto _ : state)
    {
        mb_best_match(arena->free_root, rand() % Kilobyte(1));
    }

    destroy_mem_arena(arena);
}
BENCHMARK(bm_rbt_search)->Iterations(9000);

BENCHMARK_MAIN();
