#pragma once
#include <types/types.h>

namespace Vultr
{
    namespace Platform
    {
        struct PlatformMemoryBlock;
        void *get_memory(PlatformMemoryBlock *block);
        size_t get_memory_size(PlatformMemoryBlock *block);
        PlatformMemoryBlock *virtual_alloc(void *address_hint, size_t length);
        void virtual_free(PlatformMemoryBlock *block);
    } // namespace Platform
} // namespace Vultr
