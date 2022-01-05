#pragma once
#include "vultr_memory_internal.h"
#include "linear.h"
#include "pool.h"
#include "free_list.h"
#include "slab.h"
#include "types/static_details.h"

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

namespace Private
{
	template <typename Allocator>
	void *malloc(Allocator *allocator, size_t size);

	template <typename Allocator>
	void *realloc(Allocator *allocator, void *memory, size_t size);

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
template <typename Allocator, typename T, typename... Args>
requires(!Vultr::is_same<T, void>) T *v_alloc_safe(Allocator *allocator, Args... args, size_t count = 1)
{
	T *buf = static_cast<T *>(Private::malloc(allocator, count * sizeof(T)));
	if (buf == nullptr)
		return nullptr;

	return new (buf) T(args...);
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
template <typename Allocator, typename T, typename... Args>
requires(!Vultr::is_same<T, void>) T *v_alloc(Allocator *allocator, Args... args, size_t count = 1)
{
	T *buf = v_alloc_safe<Allocator, T>(allocator, args..., count);
	PRODUCTION_ASSERT(buf != nullptr, "Failed to allocate memory!");
	return buf;
}

/**
 * Reallocate a block of memory using an allocator.
 * This does not throw an exception if it fails to reallocate, handling this is the responsibility of the call-site
 *
 * @param Allocator *allocator: The memory allocator.
 * @param T *memory: The memory that was allocated.
 * @param size_t count: The number of blocks of memory of the requested size to allocate.
 *
 * @return T *: The reallocated memory.
 *
 * @error This will return nullptr if it fails to allocate.
 */
// TODO(Brandon): Add copy constructor and destructor safety.
template <typename Allocator, typename T>
requires(!Vultr::is_same<T, void>) T *v_realloc_safe(Allocator *allocator, T *memory, size_t count) { return static_cast<T *>(Private::realloc(allocator, memory, count * sizeof(T))); }

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
template <typename Allocator, typename T>
requires(!Vultr::is_same<T, void>) T *v_realloc(Allocator *allocator, T *memory, size_t count)
{
	T *new_buf = v_realloc_safe<Allocator, T>(allocator, memory, count);
	PRODUCTION_ASSERT(new_buf != nullptr, "Failed to reallocate memory!");

	return new_buf;
}

/**
 * Free a block of memory using an allocator and call destructor.
 *
 * @param Allocator *allocator: The memory allocator.
 * @param T *memory: The memory that was allocated.
 */
template <typename Allocator, typename T>
void v_free(Allocator *allocator, T *memory)
{
	if constexpr (!Vultr::is_same<T, void>)
		// Call destructor.
		memory->~T();
	Private::free(allocator, memory);
}
