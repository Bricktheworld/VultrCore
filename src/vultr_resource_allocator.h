#pragma once
#include <core/resource_allocator/resource_allocator.h>
#include <platform/rendering.h>
#include <ecs/component.h>
#include <core/components/material.h>

namespace Vultr
{
	using ResourceTypes = TypeList<Platform::Texture *, Platform::Mesh *, Platform::Shader *, Platform::ComputeShader *, Platform::Material *>;

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
	inline void ResourceAllocator<Platform::Shader *>::notify_reload(const UUID &id)
	{
		Platform::Lock lock(mutex);
		if (!resources.contains(id))
			return;

		auto *info = &resources.get(id);
		if (info->load_state == ResourceLoadState::LOADED)
		{
			info->load_state  = ResourceLoadState::NEED_TO_LOAD;
			info->error       = None;

			auto *shader_data = info->data;
			info->data        = nullptr;

			Vector<UUID> mat_uuids_to_reload{};
			auto *mat_allocator = resource_allocator<Platform::Material *>();
			{
				Platform::Lock l(mat_allocator->mutex);

				for (auto [mat_id, mat_info] : mat_allocator->resources)
				{
					auto *mat = mat_info.data;
					if (mat == nullptr || mat->source.id != id)
						continue;
					mat_uuids_to_reload.push_back(mat_id);
				}
			}

			for (auto &mat_id : mat_uuids_to_reload)
			{
				mat_allocator->notify_reload(mat_id);
			}

			while (!mat_allocator->free_queue.empty())
			{
				mat_allocator->free_queue.queue_cond.wait(lock);
			}

			free_queue.push(shader_data);
			if (free_queue_listener != nullptr)
			{
				free_queue_listener->push(shader_data);
			}
		}
		else
		{
			info->load_state = ResourceLoadState::NEED_TO_LOAD;
			info->error      = None;
			info->data       = nullptr;
		}
		load_queue.push(id);
		return;
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
