#pragma once
#include <types/types.h>
#include "vultr_memory_internal.h"

namespace Vultr
{
	struct FreeListMemoryBlock;

	/**
	 * Memory allocator that can allocate any size with the least fragmentation possible.
	 */
	struct FreeListAllocator : public Allocator
	{
		// TODO(Brandon): Add support for 32 bit alignment (8 bytes)
		u8 alignment                    = 16;
		FreeListMemoryBlock *free_root  = nullptr;
		FreeListMemoryBlock *block_head = nullptr;
		size_t used                     = 0;

		FreeListAllocator() : Allocator(AllocatorType::FreeList) {}
	};

	/**
	 * Initialize a new free list allocator. This allocator is best used for infrequent but large allocations.
	 * This allocator reduces memory fragmentation but is very slow.
	 *
	 * @param MemoryArena *arena: The memory arena to create the allocator from.
	 * @param size_t size: The total size that this allocator will be able to allocate in bytes.
	 * @param u8 alignment: The alignment of the allocator in bytes.
	 *
	 * @return FreeListAllocator *: The allocator which can be now be used.
	 *
	 * @error The method will return nullptr if the memory arena has run out of memory and cannot allocate.
	 *
	 * @no_thread_safety
	 */
	FreeListAllocator *init_free_list_allocator(MemoryArena *arena, size_t size, u8 alignment);

	/**
	 * Allocate a chunk of memory using a free list allocator.
	 *
	 * @param FreeListAllocator *allocator: The allocator to use.
	 * @param size_t size: The size of memory to allocate.
	 *
	 * @return void *: The memory that can now be used.
	 *
	 * @error The method will return nullptr if there is no memory chunk available to allocate.
	 *
	 * @no_thread_safety
	 */
	void *free_list_alloc(FreeListAllocator *allocator, size_t size);

	/**
	 * Resize a chunk of memory from a `FreeListAllocator`.
	 *
	 * @param FreeListAllocator *allocator: The allocator to use.
	 * @param void *: The old allocated block of memory.
	 * @param size_t size: The new size of memory to allocate.
	 *
	 * @return void *: The memory that can now be used.
	 *
	 * @error The method will return nullptr if there is no memory chunk available to allocate.
	 *
	 * @no_thread_safety
	 */
	void *free_list_realloc(FreeListAllocator *allocator, void *data, size_t size);

	/**
	 * Free a chunk of memory from a `FreeListAllocator`.
	 * @param FreeListAllocator *allocator: The allocator to use.
	 * @param void *data: The data to free.
	 *
	 * @no_thread_safety
	 */
	void free_list_free(FreeListAllocator *allocator, void *data);

} // namespace Vultr
