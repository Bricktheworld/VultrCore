#pragma once
#include <types/types.h>

namespace Vultr
{
    struct MemoryArena
    {
    };

    struct GameMemory
    {
        u64 permanent_storage_size = 0;
        void *permanent_storage = nullptr;

        u64 temporary_storage_size = 0;
        void *temporary_storage = nullptr;
    };

    GameMemory *game_mem_alloc(u64 perm_storage_size, u64 temp_storage_size);
    void game_mem_free(GameMemory *game_mem);

} // namespace Vultr
