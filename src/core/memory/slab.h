#pragma once
#include "memory_impl.h"
#include <platform/platform.h>

namespace Vultr
{
	struct Slab;
	struct SlabAllocator : Allocator
	{
		size_t max_slab_size = 0;
		Slab **slabs         = nullptr;
		u32 num_slabs        = 0;
		Platform::Mutex mutex{};

		SlabAllocator() : Allocator(AllocatorType::Slab) {}
	};

	struct SlabDeclaration
	{
		u32 block_size;
		u32 count;
	};

	// NOTE(Brandon): Expects declarations sorted ascending in block size.
	SlabAllocator *init_slab_allocator(MemoryArena *arena, SlabDeclaration *declarations, u32 region_count);

	void *slab_alloc(SlabAllocator *allocator, size_t size);

	size_t slab_get_size(SlabAllocator *allocator, void *data);

	void *slab_realloc(SlabAllocator *allocator, void *data, size_t size);

	void slab_free(SlabAllocator *allocator, void *data);
} // namespace Vultr