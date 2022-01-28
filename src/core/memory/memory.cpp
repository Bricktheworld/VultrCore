#include "memory.h"
#include "linear.cpp"
#include "pool.cpp"
#include "free_list.cpp"
#include "slab.cpp"
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
			arena->next_free_chunk = reinterpret_cast<byte *>(chunk) + size;
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

	void *mem_alloc(Allocator *allocator, size_t size)
	{
		ASSERT(allocator != nullptr, "Cannot allocate from an invalid memory allocator!");
		switch (allocator->type)
		{
			case AllocatorType::Linear:
				return linear_alloc(reinterpret_cast<LinearAllocator *>(allocator), size);
			case AllocatorType::Pool:
				return pool_alloc(reinterpret_cast<PoolAllocator *>(allocator), size);
			case AllocatorType::FreeList:
				return free_list_alloc(reinterpret_cast<FreeListAllocator *>(allocator), size);
			case AllocatorType::Slab:
				return slab_alloc(reinterpret_cast<SlabAllocator *>(allocator), size);
			case AllocatorType::Stack:
				NOT_IMPLEMENTED("Stack has not yet been implemented in the engine :(");
			case AllocatorType::None:
			default:
				THROW("Invalid memory allocator, how the fuck did you even get here.");
		}
	}

	void *mem_realloc(Allocator *allocator, void *memory, size_t size)
	{
		switch (allocator->type)
		{
			case AllocatorType::Linear:
				THROW("Cannot reallocate in a linear allocator, the entire point of linear is to not do that.");
			case AllocatorType::Pool:
				return pool_realloc(reinterpret_cast<PoolAllocator *>(allocator), memory, size);
			case AllocatorType::FreeList:
				return free_list_realloc(reinterpret_cast<FreeListAllocator *>(allocator), memory, size);
			case AllocatorType::Slab:
				return slab_realloc(reinterpret_cast<SlabAllocator *>(allocator), memory, size);
			case AllocatorType::Stack:
				THROW("Cannot reallocate in a stack allocator, this is not what a stack allocator is for.");
			case AllocatorType::None:
			default:
				THROW("Invalid memory allocator, how the fuck did you even get here.");
		}
	}

	void mem_free(Allocator *allocator, void *memory)
	{
		switch (allocator->type)
		{
			case AllocatorType::Linear:
				THROW("You cannot free individual blocks of memory from a linear allocator.");
			case AllocatorType::Pool:
				pool_free(reinterpret_cast<PoolAllocator *>(allocator), memory);
				break;
			case AllocatorType::FreeList:
				free_list_free(reinterpret_cast<FreeListAllocator *>(allocator), memory);
				break;
			case AllocatorType::Slab:
				slab_free(reinterpret_cast<SlabAllocator *>(allocator), memory);
				break;
			case AllocatorType::Stack:
				NOT_IMPLEMENTED("Stack has not yet been implemented in the engine :(");
			case AllocatorType::None:
			default:
				THROW("Invalid memory allocator, how the fuck did you even get here.");
		}
	}

	void *persist_alloc(size_t size) { return linear_alloc(g_game_memory->persistent_storage, size); }

} // namespace Vultr

namespace Private
{
	template <>
	void *malloc<Vultr::LinearAllocator>(Vultr::LinearAllocator *allocator, size_t size)
	{
		return Vultr::linear_alloc(allocator, size);
	}

	template <>
	void *malloc(Vultr::PoolAllocator *allocator, size_t size)
	{
		return Vultr::pool_alloc(allocator, size);
	}

	template <>
	void *malloc(Vultr::FreeListAllocator *allocator, size_t size)
	{
		return Vultr::free_list_alloc(allocator, size);
	}

	template <>
	void *malloc(Vultr::SlabAllocator *allocator, size_t size)
	{
		return Vultr::slab_alloc(allocator, size);
	}

	template <>
	void *realloc(Vultr::PoolAllocator *allocator, void *memory, size_t size)
	{
		return Vultr::pool_realloc(allocator, memory, size);
	}

	template <>
	void *realloc(Vultr::FreeListAllocator *allocator, void *memory, size_t size)
	{
		return Vultr::free_list_realloc(allocator, memory, size);
	}

	template <>
	void *realloc(Vultr::SlabAllocator *allocator, void *memory, size_t size)
	{
		return Vultr::slab_realloc(allocator, memory, size);
	}

	template <>
	void free(Vultr::PoolAllocator *allocator, void *memory)
	{
		return Vultr::pool_free(allocator, memory);
	}

	template <>
	void free(Vultr::FreeListAllocator *allocator, void *memory)
	{
		return Vultr::free_list_free(allocator, memory);
	}

	template <>
	void free(Vultr::SlabAllocator *allocator, void *memory)
	{
		return Vultr::slab_free(allocator, memory);
	}
} // namespace Private
