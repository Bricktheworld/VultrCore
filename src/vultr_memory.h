#pragma once
#include "core/vultr_core.h"

namespace Vultr
{
	/**
	 * Holds memory allocators that are used throughout the program.
	 */
	struct GameMemory
	{
		MemoryArena *arena                   = nullptr;
		LinearAllocator *persistent_storage  = nullptr;
		LinearAllocator *frame_storage       = nullptr;
		FreeListAllocator *general_allocator = nullptr;
		SlabAllocator *slab_allocator        = nullptr;
		PoolAllocator *pool_allocator        = nullptr;
		void *world                          = nullptr;
		void *engine                         = nullptr;
	};

	extern GameMemory *g_game_memory;

	GameMemory *init_game_memory();
	void destroy_game_memory(GameMemory *m);

	typedef void (*UseGameMemoryApi)(GameMemory *m);

	// TODO(Brandon): Update these with actual parameters.
	typedef void (*VultrInitApi)(void);
	typedef void (*VultrUpdateApi)(void);

	constexpr const char *USE_GAME_MEMORY_SYMBOL = "vultr_init";
	constexpr const char *VULTR_INIT_SYMBOL      = "vultr_init";
	constexpr const char *VULTR_UPDATE_SYMBOL    = "vultr_update";

} // namespace Vultr

VULTR_API void use_game_memory(void *m);
VULTR_API void vultr_init(void);
VULTR_API void vultr_update(void);

/**
 * Allocate some memory using game memory and call constructor.
 *
 * @param size_t count: The number of blocks of memory of the requested size to allocate.
 *
 * @return T *: The newly allocated memory.
 *
 * @error This will crash the program if the memory failed to allocate.
 */
template <typename T, typename... Args>
requires(!Vultr::is_same<T, void>) T *v_alloc(Args... args, size_t count = 1)
{
	ASSERT(Vultr::g_game_memory != nullptr, "Cannot allocate using `v_alloc(Args... args, size_t count)` without first calling `init_game_memory()`. \n"
											"Use `v_alloc(Allocator *allocator, Args... args, size_t count)` if you would like to use an allocator.");
	auto *slab_allocator      = Vultr::g_game_memory->slab_allocator;
	auto *free_list_allocator = Vultr::g_game_memory->general_allocator;

	if (sizeof(T) * count <= slab_allocator->max_slab_size)
	{
		auto *data = v_alloc_safe<Vultr::SlabAllocator, T>(slab_allocator, args..., count);
		if (data != nullptr)
			return data;
	}
	return v_alloc<Vultr::FreeListAllocator, T>(free_list_allocator, args..., count);
}

/**
 * Reallocate a block of memory using game memory.
 *
 * @param T *memory: The memory that was allocated.
 * @param size_t count: The number of blocks of memory of the requested size to allocate.
 *
 * @return T *: The reallocated memory.
 *
 * @error This will crash the program if it failed to reallocate.
 */
template <typename T>
requires(!Vultr::is_same<T, void>) T *v_realloc(T *memory, size_t count)
{
	ASSERT(Vultr::g_game_memory != nullptr, "Cannot allocate using `realloc(T *memory, size_t count)` without first calling `init_game_memory()`. \n"
											"Use `realloc(Allocator *allocator, T *memory, size_t count)` if you would like to use an allocator.");
	auto *slab_allocator      = Vultr::g_game_memory->slab_allocator;
	auto *free_list_allocator = Vultr::g_game_memory->general_allocator;

	// If the memory resides in the slab allocator.
	if (reinterpret_cast<byte *>(memory) > reinterpret_cast<byte *>(slab_allocator))
	{
		if (sizeof(T) * count <= slab_allocator->max_slab_size)
		{
			auto *data = v_realloc_safe<Vultr::SlabAllocator, T>(slab_allocator, memory, count);
			if (data != nullptr)
			{
				return data;
			}
		}
		v_free<Vultr::SlabAllocator, T>(slab_allocator, memory);
		return v_alloc<Vultr::FreeListAllocator, T>(free_list_allocator, count);
	}
	return v_realloc<Vultr::FreeListAllocator, T>(free_list_allocator, memory, count);
}

/**
 * Free a block of memory using game memory.
 *
 * @param T *memory: The memory that was allocated.
 */
template <typename T>
void v_free(T *memory)
{
	ASSERT(Vultr::g_game_memory != nullptr, "Cannot free using `free(T *memory)` without first calling `init_game_memory()`. \n"
											"Use `free(Allocator *allocator, T *memory)` if you would like to use an allocator.");
	auto *slab_allocator      = Vultr::g_game_memory->slab_allocator;
	auto *free_list_allocator = Vultr::g_game_memory->general_allocator;

	// If the memory resides in the slab allocator.
	if (reinterpret_cast<byte *>(memory) > reinterpret_cast<byte *>(slab_allocator))
	{
		v_free<Vultr::SlabAllocator, T>(slab_allocator, memory);
	}
	else
	{
		v_free<Vultr::FreeListAllocator>(free_list_allocator, memory);
	}
}
