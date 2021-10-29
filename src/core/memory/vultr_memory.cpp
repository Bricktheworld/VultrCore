#include "vultr_memory.h"
namespace Vultr
{
    GameMemory *game_mem_alloc(u64 perm_storage_size, u64 temp_storage_size)
    {
        auto *mem = static_cast<GameMemory *>(malloc(sizeof(GameMemory)));
        if (mem == nullptr)
        {
            return nullptr;
        }

        mem->permanent_storage_size = perm_storage_size;
        mem->permanent_storage = malloc(perm_storage_size);
        if (mem->permanent_storage == nullptr)
        {
            free(mem);
            return nullptr;
        }

        mem->temporary_storage_size = temp_storage_size;
        mem->temporary_storage = malloc(temp_storage_size);
        if (mem->temporary_storage == nullptr)
        {
            free(mem->permanent_storage);
            free(mem);
            return nullptr;
        }

        return mem;
    }
    void game_mem_free(GameMemory *game_mem)
    {
        free(game_mem->permanent_storage);
        free(game_mem->temporary_storage);
        free(game_mem);
    }
} // namespace Vultr
