#include "vultr.h"
namespace Vultr
{
    GameMemory *g_game_memory = nullptr;

    // TODO(Brandon): Add more allocators.
    GameMemory *init_game_memory()
    {
        auto *arena = init_mem_arena(Gigabyte(1));
        auto *persistent_storage    = init_linear_allocator(arena, Kilobyte(1));

        auto *game_memory = alloc<GameMemory>(persistent_storage);

        game_memory->arena = arena;
        game_memory->persistent_storage = persistent_storage;

        return game_memory;
    }


    void destroy_game_memory(GameMemory *m)
    {
        ASSERT(m != nullptr && m->arena != nullptr, "GameMemory not properly initialized!");
        destroy_mem_arena(m->arena);
    }
} // namespace Vultr
