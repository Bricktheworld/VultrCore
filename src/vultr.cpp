#include "vultr.h"
namespace Vultr
{
    // TODO(Brandon): See how this global variable plays on windows with dlls.
    static GameMemory *game_memory = nullptr;

    MemoryArena *get_memory_arena(bool threadsafe)
    {
        ASSERT(game_memory != nullptr, "Game memory has not been initialized!");
        if (!threadsafe)
        {
            return game_memory->main_heap;
        }
        else
        {
            // TODO(Brandon): Implement this.
            NOT_IMPLEMENTED("Need to implement threads first.");
            return nullptr;
        }
    }

} // namespace Vultr
