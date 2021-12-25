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
	void *malloc(Allocator *allocator, size_t size);

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
	void *mrealloc(Allocator *allocator, void *memory, size_t size);

	/**
	 * Free a block of memory using an allocator.
	 *
	 * @param Allocator *allocator: The memory allocator.
	 * @param void *memory: The memory that was allocated.
	 */
	void mfree(Allocator *allocator, void *memory);

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
	void *persist_alloc(size_t size);

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
	void *frame_alloc(size_t size);

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
	void *pool_alloc(size_t size);

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
	void *pool_realloc(void *memory, size_t size);

	/**
	 * Free a block of memory using an allocator.
	 *
	 * @param Allocator *allocator: The memory allocator.
	 * @param void *memory: The memory that was allocated.
	 */
	void pool_free(void *memory);

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
	T *alloc(Allocator *allocator, size_t count = 1, Args... args)
	{
		T *buf = static_cast<T *>(malloc(allocator, count * sizeof(T)));
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
	T *realloc(Allocator *allocator, T *memory, size_t count)
	{
		void *new_buf = nullptr;
		switch (allocator->type)
		{
			case AllocatorType::Linear:
				THROW("Cannot reallocate in a linear allocator, the entire point of linear is to not do that.");
				break;
			case AllocatorType::Pool:
				new_buf = pool_realloc(static_cast<PoolAllocator *>(allocator), memory, sizeof(T) * count);
				break;
			case AllocatorType::FreeList:
				new_buf = free_list_realloc(static_cast<FreeListAllocator *>(allocator), memory, sizeof(T) * count);
				break;
			case AllocatorType::Stack:
				THROW("Cannot reallocate in a stack allocator, this is not what a stack allocator is for.");
				break;
			case AllocatorType::None:
			default:
				THROW("Invalid memory allocator, how the fuck did you even get here.");
		}
		PRODUCTION_ASSERT(new_buf != nullptr, "Failed to reallocate memory!");
		return new_buf;
	}

	/**
	 * Free a block of memory using an allocator and call destructor.
	 *
	 * @param Allocator *allocator: The memory allocator.
	 * @param T *memory: The memory that was allocated.
	 */
	template <typename T>
	void free(Allocator *allocator, T *memory)
	{
		// Call destructor.
		memory.~T();
		mfree(allocator, memory);
	}

	/**
	 * Class that can be inherited from to show dependency on some external allocator.
	 */
	struct MemoryStruct
	{
		MemoryStruct(Allocator *allocator) : allocator(allocator) { ASSERT(allocator != nullptr, "Memory struct must be initalized with a valid memory allocator!"); }
		Allocator *allocator = nullptr;

		/**
		 * Shorthand for a memory struct to allocate a block of memory using @ref malloc(Allocator *, size_t count, Args... args)
		 *
		 * @param size_t count: The number of blocks of memory of the requested size to allocate.
		 *
		 * @return T *: The newly allocated memory.
		 *
		 * @error This will crash the program if the memory failed to allocate.
		 */
		template <typename T, typename... Args>
		T *alloc(size_t count, Args... args)
		{
			return alloc<T>(allocator, count, args...);
		}

		/**
		 * Shorthand for a memory struct to reallocate a block of memory using @ref realloc(Allocator *, T *memory)
		 *
		 * @param T *memory: The memory that was allocated.
		 * @param size_t count: The number of blocks of memory of the requested size to allocate.
		 *
		 * @return T *: The reallocated memory.
		 *
		 * @error This will crash the program if it failed to reallocate.
		 */
		template <typename T>
		T *realloc(T *memory, size_t count)
		{
			return realloc<T>(allocator, memory, count);
		}

		/**
		 * Shorthand for a memory struct to free a block of memory using @ref realloc(Allocator *)
		 *
		 * @param T *memory: The memory that was allocated.
		 */
		template <typename T>
		void free(T *memory)
		{
			free<T>(allocator, memory);
		}
	};
} // namespace Vultr
