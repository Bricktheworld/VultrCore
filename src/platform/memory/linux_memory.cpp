#include <types/types.h>
#include "../platform_impl.h"
#include <sys/mman.h>

namespace Vultr
{
	namespace Platform
	{
		struct PlatformMemoryBlock
		{
			size_t size;
		};

		void *get_memory(PlatformMemoryBlock *block)
		{
			ASSERT(block != nullptr, "Cannot get the memory from an invalid memory block.");
			return block + 1;
		}

		size_t get_memory_size(PlatformMemoryBlock *block)
		{

			ASSERT(block != nullptr, "Cannot get the size of an invalid memory block.");
			return block->size;
		}

		PlatformMemoryBlock *virtual_alloc(void *address_hint, size_t size)
		{
			size_t total_size = size + sizeof(PlatformMemoryBlock);

			// TODO(Brandon): Handle some flags
			void *memory = mmap(address_hint, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

			// If the allocation failed.
			if (memory == (void *)-1)
				return nullptr;

			auto *block = reinterpret_cast<PlatformMemoryBlock *>(memory);
			block->size = total_size;

			return block;
		}

		void virtual_free(PlatformMemoryBlock *block)
		{
			ASSERT(block != nullptr, "Cannot free an invalid memory block.");
			auto size = block->size;
			// TODO(Brandon): Handle some flags
			munmap(block, size);
		}
	} // namespace Platform
} // namespace Vultr
