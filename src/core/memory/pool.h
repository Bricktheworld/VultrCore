#pragma once
#include <types/types.h>
#include "memory_impl.h"

namespace Vultr
{
	struct PoolSegment;

	/**
	 * Allocator that has segment(s) containing memory blocks of the same size which can be allocated.
	 */
	struct PoolAllocator : public Allocator
	{
		PoolSegment *segments = nullptr;
		u32 num_segments      = 0;

		PoolAllocator() : Allocator(AllocatorType::Pool) {}
	};

	/**
	 * Describes a section of a pool allocator that has a certain chunk size and the number of chunks available to be allocated.
	 */
	struct PoolRegion
	{
		u32 size;
		u32 count;
	};

	/**
	 * Initialize a new pool allocator. This allocator is best used for frequent but small allocations or frequent allocations of one specific type of object.
	 * This allocator has fairly large memory fragmentation but is very fast. This is best used as a "general purpose" heap allocator, for things like strings and small buffers.
	 *
	 * @param MemoryArena *arena: The memory arena to create the allocator from.
	 * @param u32 allocation_size: The size of all elements in the pool.
	 * @param u32 count: The maximum number of elements that can be allocated at one time.
	 *
	 * @return PoolAllocator *: The allocator which can be now be used.
	 *
	 * @error The method will return nullptr if the memory arena has run out of memory and cannot allocate.
	 *
	 * @no_thread_safety
	 */
	PoolAllocator *init_pool_allocator(MemoryArena *arena, u32 allocation_size, u32 count);

	/**
	 * Initialize a new pool allocator. This allocator is best used for frequent but small allocations or frequent allocations of one specific type of object.
	 * This allocator has fairly large memory fragmentation but is very fast. This is best used as a "general purpose" heap allocator, for things like strings and small buffers.
	 *
	 * @param MemoryArena *arena: The memory arena to create the allocator from.
	 * @param PoolRegion *regions: An array that tells the pool allocator the different sizes of elements that can be allocated, each of these sizes is known as a `region` or `segment`.
	 * This parameter is best created on the stack.
	 * @param u32 region_count: The number of regions that exist in this pool allocator.
	 *
	 * @return PoolAllocator *: The allocator which can be now be used.
	 *
	 * @error The method will return nullptr if the memory arena has run out of memory and cannot allocate.
	 *
	 * @no_thread_safety
	 */
	PoolAllocator *init_pool_allocator(MemoryArena *arena, PoolRegion *regions, u32 region_count);

	/**
	 * Allocate a chunk of memory using a pool allocator.
	 *
	 * @param PoolAllocator *allocator: The allocator to use.
	 * @param size_t size: The size of memory to allocate.
	 *
	 * @return void *: The memory that can now be used.
	 *
	 * @error The method will return nullptr if there is no memory chunk available to allocate.
	 *
	 * @no_thread_safety
	 */
	void *pool_alloc(PoolAllocator *allocator, size_t size);

	/**
	 * Free a chunk of memory from a `PoolAllocator`.
	 * @param PoolAllocator *allocator: The allocator to use.
	 * @param void *data: The data to free.
	 *
	 * @no_thread_safety
	 */
	void pool_free(PoolAllocator *allocator, void *data);
} // namespace Vultr
