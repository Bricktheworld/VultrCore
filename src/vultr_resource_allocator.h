#pragma once
#include <core/resource_allocator/resource_allocator.h>
#include <platform/rendering.h>

namespace Vultr
{
	using ResourceTypes = TypeList<Platform::Texture *, Platform::Mesh *, Platform::Shader *, Platform::Material *>;

	template <typename T>
	inline ResourceAllocator<T> *resource_allocator()
	{
		ASSERT(g_game_memory != nullptr && g_game_memory->resource_allocator != nullptr, "Game memory not properly initialized!");
		auto *location = reinterpret_cast<ResourceAllocator<void *> *>(g_game_memory->resource_allocator) + ResourceTypes::index_of<T>();
		return reinterpret_cast<ResourceAllocator<T> *>(location);
	}

	template <>
	inline bool Resource<Platform::Material *>::loaded() const
	{
		if (!id.has_value() || !resource_allocator<Platform::Material *>()->is_loaded(id.value()))
			return false;

		for (auto &sampler : value()->samplers)
		{
			if (!sampler.empty() && !sampler.loaded())
				return false;
		}
		return true;
	}
} // namespace Vultr
