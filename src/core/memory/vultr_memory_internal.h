#pragma once
#include <types/types.h>
#include <string.h>

namespace Vultr
{
	/**
	 * The different types of allocators that can be used.
	 */
	enum struct AllocatorType : u8
	{
		None     = 0x0,
		Linear   = 0x1,
		Stack    = 0x2,
		Pool     = 0x4,
		Slab     = 0x8,
		FreeList = 0x10,
	};

	/**
	 * Base class that all allocators inherit from.
	 */
	struct Allocator
	{
		Allocator(AllocatorType type) : type(type) {}
		AllocatorType type;
	};

#ifndef MAX_ALLOCATORS
	/**
	 * The maximum number of different types of allocators that can be used in a memory arena.
	 * Technically this is just the maximum number of sections within a memory arena available.
	 */
#define MAX_ALLOCATORS 16
#endif

	namespace Platform
	{
		struct PlatformMemoryBlock;
	}

	/**
	 * An arena of memory allocated from the OS that can be used for allocators throughout the program.
	 * This avoids kernel-user-space switching along with other performance benefits.
	 */
	struct MemoryArena
	{
		Platform::PlatformMemoryBlock *memory = nullptr;
		AllocatorType allocator_types[MAX_ALLOCATORS];
		void *allocators[MAX_ALLOCATORS];
		u32 next_index        = 0;
		void *next_free_chunk = nullptr;
	};

	/**
	 * Allocate a chunk of memory from the OS and put it in a `MemoryArena`.
	 * @param size_t size: Size in bytes of how much space the memory arena should have.
	 *
	 * @return MemoryArena *: The memory arena object.
	 *
	 * @error The method will return nullptr if it is unable to allocate the memory arena. This should be handled properly.
	 *
	 * @thread_safe
	 */
	MemoryArena *init_mem_arena(size_t size, u8 alignment = 16);

	/**
	 * Designate a section within a memory arena for a certain type of allocator.
	 *
	 * @param MemoryArena *arena: The memory arena to use.
	 * @param AllocatorType type: The type of allocator to use in this section.
	 * @param size_t size: The size in bytes of the memory allocator. This includes any header information the allocator needs.
	 *
	 * @return void *: The new memory allocator starting pointer.
	 *
	 * @error Returns nullptr if there isn't enough memory to allocate a new allocator of the requested size.
	 *
	 * @no_thread_safety
	 */
	void *mem_arena_designate(MemoryArena *arena, AllocatorType type, size_t size);

	void *mem_arena_get_allocator(MemoryArena *arena, void *data);

	/**
	 * Free a `MemoryArena` from the OS.
	 * @param MemoryArena *mem: The memory arena to be freed.
	 *
	 * @errror The method will assert if a nullptr memory arena is provided.
	 *
	 * @thread_safe
	 */
	void destroy_mem_arena(MemoryArena *arena);
} // namespace Vultr
