#pragma once
#include "core/vultr_core.h"

namespace Vultr
{

#define THREAD_SAFE_ARENAS 8
    struct GameMemory
    {
        MemoryArena *arena = nullptr;
    };

    typedef GameMemory *M;

    M init_game_memory();

    void destroy_game_memory(M m);

} // namespace Vultr
