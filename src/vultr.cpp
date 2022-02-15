#include "vultr.h"
namespace Vultr
{
	GameMemory *g_game_memory = nullptr;

	GameMemory *init_game_memory()
	{
		auto *arena = init_mem_arena(Gigabyte(1));
		PRODUCTION_ASSERT(arena != nullptr, "Not enough memory to initialize game memory! At least 1 gigabyte of memory is required.");

		auto *persistent_storage = init_linear_allocator(arena, Kilobyte(10));
		ASSERT(persistent_storage != nullptr, "Failed to allocate persistent storage linear allocator!");

		auto *game_memory               = v_alloc_with_allocator<LinearAllocator, GameMemory>(persistent_storage);

		game_memory->arena              = arena;
		game_memory->persistent_storage = persistent_storage;

		game_memory->frame_storage      = init_linear_allocator(arena, Megabyte(20));
		ASSERT(game_memory->frame_storage != nullptr, "Failed to allocate frame linear allocator!");

		game_memory->general_allocator = init_free_list_allocator(arena, Megabyte(900), 16);
		ASSERT(game_memory->general_allocator != nullptr, "Failed to allocate free list allocator!");

		SlabDeclaration declarations[] = {
			{.block_size = 64, .count = 1024},          // 64 kilobytes
			{.block_size = 128, .count = 1024},         // 128 kilobytes
			{.block_size = 256, .count = 1024},         // 256 kilobytes
			{.block_size = 512, .count = 1024},         // 512 kilobytes
			{.block_size = Kilobyte(1), .count = 1024}, // 1 megabyte
			{.block_size = Kilobyte(2), .count = 1024}, // 2 megabytes
			{.block_size = Kilobyte(4), .count = 512},  // 2 megabytes
		};
		game_memory->slab_allocator = init_slab_allocator(arena, declarations, sizeof(declarations) / sizeof(SlabDeclaration));
		ASSERT(game_memory->slab_allocator != nullptr, "Failed to allocate slab allocator!");

		return game_memory;
	}
	void destroy_game_memory(GameMemory *m)
	{
		ASSERT(m != nullptr && m->arena != nullptr, "GameMemory not properly initialized!");
		// We could go one by one and actually free each of the allocators, but this is faster, so we don't really care.
		destroy_mem_arena(m->arena);
	}

	void init()
	{
		g_game_memory         = init_game_memory();
		g_game_memory->world  = linear_alloc(g_game_memory->persistent_storage, sizeof(World));
		g_game_memory->engine = linear_alloc(g_game_memory->persistent_storage, sizeof(Engine));
		ASSERT(g_game_memory->world != nullptr, "Failed to allocate ECS world!");
		new (g_game_memory->world) World();
	}

	void destroy() { destroy_game_memory(g_game_memory); }

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
