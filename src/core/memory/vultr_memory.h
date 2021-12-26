#pragma once
#include "vultr_memory_internal.h"
#include "linear.h"
#include "pool.h"
#include "free_list.h"

namespace Vultr
{
	/**
	 * Allocate some memory using a memory arena.
	 *
	 * @param Allocator *allocator: The memory allocator.
	 * @param size_t size: The size of memory to allocate.
	 *
	 * @return void *: The newly allocated memory.
	 *
	 * @error This will return nullptr if it failed to allocate.
	 */
	void *mem_alloc(Allocator *allocator, size_t size);

	/**
	 * Reallocate a block of memory using an allocator.
	 *
	 * @param Allocator *allocator: The memory allocator.
	 * @param void *memory: The memory that was allocated.
	 * @param size_t size: The size of memory to reallocate.
	 *
	 * @return void *: The reallocated memory.
	 *
	 * @error This will return nullptr if it failed to reallocate.
	 */
	void *mem_realloc(Allocator *allocator, void *memory, size_t size);

	/**
	 * Free a block of memory using an allocator.
	 *
	 * @param Allocator *allocator: The memory allocator.
	 * @param void *memory: The memory that was allocated.
	 */
	void mem_free(Allocator *allocator, void *memory);

	/**
	 * Allocate some memory using game memory.
	 * This will use the linear allocator and will last the duration of the game memory.
	 *
	 * @param size_t size: The size of memory to allocate.
	 *
	 * @return void *: The newly allocated memory.
	 *
	 * @error This will return nullptr if it failed to allocate.
	 */
	void *persist_alloc(size_t size);

	/**
	 * Allocate some memory using game memory.
	 * This will use the linear allocator and will last the duration of the frame.
	 * The memory will get automatically freed along with the rest of the frame memory at the end of the frame automatically.
	 *
	 * @param size_t size: The size of memory to allocate.
	 *
	 * @return void *: The newly allocated memory.
	 *
	 * @error This will return nullptr if it failed to allocate.
	 */
	void *frame_alloc(size_t size);

	/**
	 * Allocate some memory using game memory.
	 * This will use the pool allocator and won't be freed until `pool_free`.
	 *
	 * @param size_t size: The size of memory to allocate.
	 *
	 * @return void *: The newly allocated memory.
	 *
	 * @error This will return nullptr if it failed to allocate.
	 */
	void *pool_alloc(size_t size);

	/**
	 * Reallocate a block of memory using game memory.
	 * This will use the pool allocator.
	 *
	 * @param void *memory: The memory that was allocated using `pool_alloc`.
	 * @param size_t size: The size of memory to reallocate.
	 *
	 * @return void *: The reallocated memory.
	 *
	 * @error This will return nullptr if it failed to reallocate.
	 */
	void *pool_realloc(void *memory, size_t size);

	/**
	 * Free a block of memory using game memory.
	 *
	 * @param void *memory: The memory that was allocated.
	 */
	void pool_free(void *memory);
} // namespace Vultr

/**
 * Allocate some memory using a memory arena and call constructor.
 *
 * @param Allocator *allocator: The memory allocator.
 * @param size_t count: The number of blocks of memory of the requested size to allocate.
 *
 * @return T *: The newly allocated memory.
 *
 * @error This will crash the program if the memory failed to allocate.
 */
template <typename T, typename... Args>
T *v_alloc(Vultr::Allocator *allocator, Args... args, size_t count = 1)
{
	T *buf = static_cast<T *>(Vultr::mem_alloc(allocator, count * sizeof(T)));
	PRODUCTION_ASSERT(buf != nullptr, "Failed to allocate memory!");

	return new (buf) T(args...);
}

/**
 * Reallocate a block of memory using an allocator.
 *
 * @param Allocator *allocator: The memory allocator.
 * @param T *memory: The memory that was allocated.
 * @param size_t count: The number of blocks of memory of the requested size to allocate.
 *
 * @return T *: The reallocated memory.
 *
 * @error This will crash the program if it failed to reallocate.
 */
// TODO(Brandon): Add copy constructor and destructor safety.
template <typename T>
T *v_realloc(Vultr::Allocator *allocator, T *memory, size_t count)
{
	void *new_buf = nullptr;
	switch (allocator->type)
	{
		case Vultr::AllocatorType::Linear:
			THROW("Cannot reallocate in a linear allocator, the entire point of linear is to not do that.");
			break;
		case Vultr::AllocatorType::Pool:
			new_buf = Vultr::pool_realloc(reinterpret_cast<Vultr::PoolAllocator *>(allocator), memory, sizeof(T) * count);
			break;
		case Vultr::AllocatorType::FreeList:
			new_buf = Vultr::free_list_realloc(reinterpret_cast<Vultr::FreeListAllocator *>(allocator), memory, sizeof(T) * count);
			break;
		case Vultr::AllocatorType::Stack:
			THROW("Cannot reallocate in a stack allocator, this is not what a stack allocator is for.");
			break;
		case Vultr::AllocatorType::None:
		default:
			THROW("Invalid memory allocator, how the fuck did you even get here.");
	}
	PRODUCTION_ASSERT(new_buf != nullptr, "Failed to reallocate memory!");
	return static_cast<T *>(new_buf);
}

/**
 * Free a block of memory using an allocator and call destructor.
 *
 * @param Allocator *allocator: The memory allocator.
 * @param T *memory: The memory that was allocated.
 */
template <typename T>
void v_free(Vultr::Allocator *allocator, T *memory)
{
	// Call destructor.
	memory->~T();
	Vultr::mem_free(allocator, memory);
}
