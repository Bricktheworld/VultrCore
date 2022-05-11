#pragma once
#include <core/resource_allocator/resource_allocator.h>
#include <platform/rendering.h>
#include <ecs/component.h>
#include <core/components/material.h>

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

	template <>
	inline bool Resource<Platform::Material *>::has_error() const
	{
		if (!id.has_value())
			return false;

		if (resource_allocator<Platform::Material *>()->has_error(id.value()))
			return true;

		if (resource_allocator<Platform::Material *>()->is_loaded(id.value()))
		{
			for (auto &sampler : value()->samplers)
			{
				if (!sampler.empty() && sampler.has_error())
					return true;
			}
		}

		return false;
	}
	template <>
	inline Error Resource<Platform::Material *>::get_error() const
	{
		ASSERT(id.has_value(), "Cannot get error from invalid material!");

		if (resource_allocator<Platform::Material *>()->has_error(id.value()))
			return resource_allocator<Platform::Material *>()->get_error(id.value());

		ASSERT(resource_allocator<Platform::Material *>()->is_loaded(id.value()), "Cannot get error from material which doesn't have error!");

		for (auto &sampler : value()->samplers)
		{
			if (!sampler.empty() && sampler.has_error())
				return sampler.get_error();
		}

		THROW("Failed to find sampler with error!");
	}

	template <>
	inline EditorType get_editor_type<Material>(Material *component)
	{
		EditorType type = {get_type<Material>, component};
		if (!component->source.loaded())
			return type;

		Platform::Material *mat  = component->source.value();
		Platform::Shader *shader = mat->source.value();

		const auto *reflection   = Platform::get_reflection_data(shader);

		for (auto &uniform_member : reflection->uniform_members)
		{
			auto field = Field(uniform_member.name, uniform_member.type, nullptr, [](void *data) {});
			type.fields.push_back({field, &mat->uniform_data[uniform_member.offset]});
		}

		u32 i = 0;
		for (auto &sampler : reflection->samplers)
		{
			auto field = Field(sampler.name, get_type<Resource<Platform::Texture *>>, nullptr, [](void *data) { new (data) Resource<Platform::Texture *>(); });
			type.fields.push_back({field, &mat->samplers[i]});
			i++;
		}
		return type;
	}
} // namespace Vultr
