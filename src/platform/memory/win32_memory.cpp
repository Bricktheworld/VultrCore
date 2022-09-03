#include <types/types.h>
#include "../platform_impl.h"

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
			void *memory = VirtualAlloc(address_hint, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			// If the allocation failed.
			if (memory == nullptr)
				return nullptr;

			auto *block = reinterpret_cast<PlatformMemoryBlock *>(memory);
			block->size = total_size;

			return block;
		}

		void virtual_free(PlatformMemoryBlock *block)
		{
			ASSERT(block != nullptr, "Cannot free an invliad memory block.");
			auto size = block->size;
			// TODO(Brandon): Handle some flags
			VirtualFree(block, 0, MEM_FREE);
		}
	} // namespace Platform
} // namespace Vultr
