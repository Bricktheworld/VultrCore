#include "vultr_memory.h"
#include "linear.cpp"
#include "pool.cpp"
#include "free_list.cpp"
#include <platform/platform.h>

namespace Vultr
{
	MemoryArena *init_mem_arena(size_t size, u8 alignment)
	{
		// Virtual alloc some memory.
		auto *memory_block = Platform::virtual_alloc(nullptr, sizeof(MemoryArena) + size);

		// If it returned nullptr, then we can assume that the allocation failed.
		if (memory_block == nullptr)
		{
			return nullptr;
		}

		// The memory allocator will be at the start of this platform memory block.
		auto *arena            = reinterpret_cast<MemoryArena *>(Platform::get_memory(memory_block));
		arena->memory          = memory_block;
		arena->next_free_chunk = reinterpret_cast<byte *>(arena) + sizeof(MemoryArena);
		arena->next_index      = 0;

		return arena;
	}

	void *mem_arena_designate(MemoryArena *arena, AllocatorType type, size_t size)
	{
		ASSERT(type != AllocatorType::None, "Cannot designate an invalid allocator!");

		if (arena->next_index >= MAX_ALLOCATORS)
			return nullptr;

		if (arena->next_free_chunk == nullptr)
			return nullptr;

		void *chunk           = arena->next_free_chunk;
		size_t remaining_size = Platform::get_memory_size(arena->memory) - (reinterpret_cast<byte *>(chunk) - reinterpret_cast<byte *>(arena));
		if (remaining_size >= size)
		{
			auto index                    = arena->next_index;
			arena->allocators[index]      = chunk;
			arena->allocator_types[index] = type;
			arena->next_index++;
			return chunk;
		}
		else
		{
			return nullptr;
		}
	}

	void destroy_mem_arena(MemoryArena *arena)
	{
		ASSERT(arena != nullptr && arena->memory != nullptr, "Invalid memory arena!");
		Platform::virtual_free(arena->memory);
	}

	void *malloc(Allocator *allocator, size_t size)
	{
		ASSERT(allocator != nullptr, "Cannot allocate from an invalid memory allocator!");
		switch (allocator->type)
		{
			case AllocatorType::Linear:
				return linear_alloc(static_cast<LinearAllocator *>(allocator), size);
				break;
			case AllocatorType::Pool:
				return pool_alloc(static_cast<PoolAllocator *>(allocator), size);
				break;
			case AllocatorType::FreeList:
				return free_list_alloc(static_cast<FreeListAllocator *>(allocator), size);
				break;
			case AllocatorType::Stack:
				NOT_IMPLEMENTED("Stack has not yet been implemented in the engine :(");
				break;
			case AllocatorType::None:
			default:
				THROW("Invalid memory allocator, how the fuck did you even get here.");
				return nullptr;
		}
	}

	void *mrealloc(Allocator *allocator, void *memory, size_t size)
	{
		switch (allocator->type)
		{
			case AllocatorType::Linear:
				THROW("Cannot reallocate in a linear allocator, the entire point of linear is to not do that.");
				break;
			case AllocatorType::Pool:
				return pool_realloc(static_cast<PoolAllocator *>(allocator), memory, size);
				break;
			case AllocatorType::FreeList:
				return free_list_realloc(static_cast<FreeListAllocator *>(allocator), memory, size);
				break;
			case AllocatorType::Stack:
				THROW("Cannot reallocate in a stack allocator, this is not what a stack allocator is for.");
				break;
			case AllocatorType::None:
			default:
				THROW("Invalid memory allocator, how the fuck did you even get here.");
		}
	}

	void free(Allocator *allocator, void *memory)
	{
		switch (allocator->type)
		{
			case AllocatorType::Linear:
				THROW("You cannot free individual blocks of memory from a linear allocator.");
				break;
			case AllocatorType::Pool:
				pool_free(static_cast<PoolAllocator *>(allocator), memory);
				break;
			case AllocatorType::FreeList:
				free_list_free(static_cast<FreeListAllocator *>(allocator), memory);
				break;
			case AllocatorType::Stack:
				NOT_IMPLEMENTED("Stack has not yet been implemented in the engine :(");
				break;
			case AllocatorType::None:
			default:
				THROW("Invalid memory allocator, how the fuck did you even get here.");
		}
	}

} // namespace Vultr
