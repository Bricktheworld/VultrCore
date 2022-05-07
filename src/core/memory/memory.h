#pragma once
#include "memory_impl.h"
#include "linear.h"
#include "pool.h"
#include "free_list.h"
#include "slab.h"
#include <types/static_details.h>

namespace Vultr
{
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
	 * Free a block of memory using game memory.
	 *
	 * @param void *memory: The memory that was allocated.
	 */
	void pool_free(void *memory);
} // namespace Vultr

namespace Private
{
	template <typename Allocator>
	void *malloc(Allocator *allocator, size_t size);

	template <typename Allocator>
	void free(Allocator *allocator, void *memory);
} // namespace Private

/**
 * Allocate some memory using a memory arena and call constructor.
 * This does not throw an exception if it fails to allocate, handling this is the responsibility of the call-site.
 *
 * @param Allocator *allocator: The memory allocator.
 * @param size_t count: The number of blocks of memory of the requested size to allocate.
 *
 * @return T *: The newly allocated memory.
 *
 * @error This will return nullptr if it failed to allocate.
 */
template <typename Allocator, typename T>
requires(!Vultr::is_same<T, void>) T *v_alloc_safe(Allocator *allocator, size_t count = 1)
{
	T *buf = static_cast<T *>(Private::malloc(allocator, count * sizeof(T)));
	if (buf == nullptr)
		return nullptr;

	for (size_t i = 0; i < count; i++)
		new (buf + i) T();
	return buf;
}

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
template <typename Allocator, typename T>
requires(!Vultr::is_same<T, void>) T *v_alloc_with_allocator(Allocator *allocator, size_t count = 1)
{
	T *buf = v_alloc_safe<Allocator, T>(allocator, count);
	PRODUCTION_ASSERT(buf != nullptr, "Failed to allocate memory!");
	return buf;
}

/**
 * Free a block of memory using an allocator and call destructor.
 *
 * @param Allocator *allocator: The memory allocator.
 * @param T *memory: The memory that was allocated.
 */
template <typename Allocator, typename T>
void v_free_with_allocator(Allocator *allocator, T *memory)
{
	if constexpr (!Vultr::is_same<T, void>)
		// Call destructor.
		memory->~T();
	Private::free(allocator, memory);
}
