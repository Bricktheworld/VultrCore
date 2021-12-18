#pragma once
#include "core/vultr_core.h"


namespace Vultr
{

#define THREAD_SAFE_ARENAS 8
    struct GameMemory
    {
        MemoryArena *arena = nullptr;
        LinearAllocator *persistent_storage = nullptr;
        LinearAllocator *frame_storage      = nullptr;
    };

    extern GameMemory *g_game_memory;

    GameMemory *init_game_memory();

    void destroy_game_memory(GameMemory *m);


    typedef void (*UseGameMemoryApi)(GameMemory *m);

    // TODO(Brandon): Update these with actual parameters.
    typedef void (*VultrInitApi)(void);
    typedef void (*VultrUpdateApi)(void);

} // namespace Vultr

VULTR_API void use_game_memory(void *m);
VULTR_API void vultr_init(void);
VULTR_API void vultr_update(void);
