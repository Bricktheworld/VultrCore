#pragma once

namespace Vultr
{
    namespace Platform
    {
        struct PlatformMemoryBlock;
        void *get_memory(PlatformMemoryBlock *block);
        PlatformMemoryBlock *virtual_alloc(void *address_hint, size_t length);
        void virtual_free(PlatformMemoryBlock *block);
    } // namespace Platform
} // namespace Vultr
