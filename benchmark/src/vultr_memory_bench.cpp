#include <benchmark/benchmark.h>
#include <core/memory/vultr_memory.h>

static void bm_malloc(benchmark::State &state)
{
    for (auto _ : state)
    {
        void *data = malloc(1024);
    }
}
// Register the function as a benchmark
BENCHMARK(bm_malloc);

// Define another benchmark
static void bm_arena_alloc(benchmark::State &state)
{
    using namespace Vultr;
    MemoryArena *arena = init_mem_arena(Gigabyte(2));
    for (auto _ : state)
    {
        void *data = mem_arena_alloc(arena, 1024);
    }
    destroy_mem_arena(arena);
}
BENCHMARK(bm_arena_alloc);

BENCHMARK_MAIN();
