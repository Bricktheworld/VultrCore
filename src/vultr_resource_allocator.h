#pragma once
#include <core/resource_allocator/resource_allocator.h>
#include <platform/rendering.h>

namespace Vultr
{
	using ResourceTypes = TypeList<Platform::Texture *, Platform::Mesh *, Platform::Shader *>;

	template <typename T>
	inline ResourceAllocator<T> *resource_allocator()
	{
		ASSERT(g_game_memory != nullptr && g_game_memory->resource_allocator != nullptr, "Game memory not properly initialized!");
		auto *location = reinterpret_cast<ResourceAllocator<void *> *>(g_game_memory->resource_allocator) + ResourceTypes::index_of<T>();
		return reinterpret_cast<ResourceAllocator<T> *>(location);
	}
} // namespace Vultr
