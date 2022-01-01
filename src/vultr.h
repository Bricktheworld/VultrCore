#pragma once
#include "core/vultr_core.h"

namespace Vultr
{
	struct Heap
	{
	};

	/**
	 * Holds memory allocators that are used throughout the program.
	 */
	struct GameMemory
	{
		MemoryArena *arena                   = nullptr;
		LinearAllocator *persistent_storage  = nullptr;
		LinearAllocator *frame_storage       = nullptr;
		FreeListAllocator *general_allocator = nullptr;
		PoolAllocator *pool_allocator        = nullptr;
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
T *v_alloc(Args... args, size_t count = 1)
{
	ASSERT(Vultr::g_game_memory != nullptr, "Cannot allocate using `alloc(Args... args, size_t count)` without first calling `init_game_memory()`. \n"
											"Use `alloc(Allocator *allocator, Args... args, size_t count)` if you would like to use an allocator.");
	auto *allocator = (Vultr::Allocator *)Vultr::g_game_memory->general_allocator;
	return v_alloc<T>(allocator, args..., count);
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
// TODO(Brandon): Add copy constructor and destructor safety.
template <typename T>
T *v_realloc(T *memory, size_t count)
{
	ASSERT(Vultr::g_game_memory != nullptr, "Cannot allocate using `realloc(T *memory, size_t count)` without first calling `init_game_memory()`. \n"
											"Use `realloc(Allocator *allocator, T *memory, size_t count)` if you would like to use an allocator.");
	auto *allocator = (Vultr::Allocator *)Vultr::g_game_memory->general_allocator;
	return v_realloc<T>(allocator, memory, count);
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
	auto *allocator = (Vultr::Allocator *)Vultr::g_game_memory->general_allocator;
	v_free<T>(allocator, memory);
}
