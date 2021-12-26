#include "vultr.h"
namespace Vultr
{
	GameMemory *g_game_memory = nullptr;

	GameMemory *init_game_memory()
	{
		auto *arena                     = init_mem_arena(Gigabyte(1));
		PRODUCTION_ASSERT(arena != nullptr, "Not enough memory to initialize game memory! At least 1 gigabyte of memory is required.");

		auto *persistent_storage        = init_linear_allocator(arena, Kilobyte(1));
		ASSERT(persistent_storage != nullptr, "Failed to allocate persistent storage linear allocator!");

		auto *game_memory               = v_alloc<GameMemory>(persistent_storage);

		game_memory->arena              = arena;
		game_memory->persistent_storage = persistent_storage;

		game_memory->general_allocator = init_free_list_allocator(arena, Megabyte(900), 16);
		ASSERT(game_memory->general_allocator != nullptr, "Failed to allocate free list allocator!");

		game_memory->frame_storage = init_linear_allocator(arena, Megabyte(20));
		ASSERT(game_memory->frame_storage != nullptr, "Failed to allocate frame linear allocator!");

		return game_memory;
	}

	void destroy_game_memory(GameMemory *m)
	{
		ASSERT(m != nullptr && m->arena != nullptr, "GameMemory not properly initialized!");
		// We could go one by one and actually free each of the allocators, but this is faster, so we don't really care.
		destroy_mem_arena(m->arena);
	}

} // namespace Vultr

// TODO(Brandon): Figure out if it's a good idea to make these work.
#ifdef EVIL
void *operator new(size_t size) noexcept(false)
{
	ASSERT(Vultr::g_game_memory != nullptr, "Game memory must be initialized using `init_game_memory()` before `new` operator can be called.");

	auto *buf = Vultr::mem_alloc(Vultr::g_game_memory->general_allocator, size);
	PRODUCTION_ASSERT(buf != nullptr, "Game memory free list allocator is out of memory!");
	return buf;
}

void operator delete(void *buf) noexcept
{
	ASSERT(Vultr::g_game_memory != nullptr, "Game memory must be initialized using `init_game_memory()` before `delete` operator can be called.");

	Vultr::mem_free(Vultr::g_game_memory->general_allocator, buf);
	PRODUCTION_ASSERT(buf != nullptr, "Game memory free list allocator is out of memory!");
}

void *operator new[](size_t size) noexcept(false)
{
	ASSERT(Vultr::g_game_memory != nullptr, "Game memory must be initialized using `init_game_memory()` before `new[]` operator can be called.");

	auto *buf = Vultr::mem_alloc(Vultr::g_game_memory->general_allocator, size);
	PRODUCTION_ASSERT(buf != nullptr, "Game memory free list allocator is out of memory!");
	return buf;
}

void operator delete[](void *buf) noexcept
{
	ASSERT(Vultr::g_game_memory != nullptr, "Game memory must be initialized using `init_game_memory()` before `delete[]` operator can be called.");

	Vultr::mem_free(Vultr::g_game_memory->general_allocator, buf);
	PRODUCTION_ASSERT(buf != nullptr, "Game memory free list allocator is out of memory!");
}
#endif