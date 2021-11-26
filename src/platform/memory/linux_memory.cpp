#include <types/types.h>
#include "../platform.h"
#include <sys/mman.h>

namespace Vultr
{
    namespace Platform
    {
        struct PlatformMemoryBlock
        {
            size_t size;
        };

        void *get_memory(PlatformMemoryBlock *block) { return reinterpret_cast<char *>(block) + sizeof(PlatformMemoryBlock); }

        size_t get_memory_size(PlatformMemoryBlock *block) { return block->size; }

        PlatformMemoryBlock *virtual_alloc(void *address_hint, size_t length)
        {
            // TODO(Brandon): Handle some flags
            void *memory = mmap(address_hint, length + sizeof(PlatformMemoryBlock), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (memory == (void *)-1)
                return nullptr;
            auto *block = reinterpret_cast<PlatformMemoryBlock *>(memory);
            block->size = length + sizeof(PlatformMemoryBlock);
            return block;
        }
        void virtual_free(PlatformMemoryBlock *block)
        {
            auto size = block->size;
            // TODO(Brandon): Handle some flags
            munmap(block, size);
        }
    } // namespace Platform
} // namespace Vultr
