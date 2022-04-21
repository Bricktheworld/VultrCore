#pragma once
#include "ecs/world.h"
#include "core/components/components.h"
#include <math/decompose_transform.h>

namespace Vultr
{
	using World = WorldT<Mesh, Transform, Material, Camera, DirectionalLight>;

	inline World *world()
	{
		ASSERT(g_game_memory != nullptr && g_game_memory->world != nullptr, "Game memory not properly initialized!");
		return static_cast<World *>(g_game_memory->world);
	}
	template <typename T>
	inline ErrorOr<void> try_add_component(Entity entity, const T &component)
	{
		auto res = world()->component_manager.template try_add_component(entity, component);
		if (res.has_error())
			return res.get_error();

		auto signature            = world()->component_manager.template signature_from_components<T>();
		const auto &old_signature = world()->entity_manager.get_signature(entity);

		world()->entity_manager.add_signature(entity, signature);
		world()->system_manager.entity_signature_changed(entity, old_signature, signature);
		return Success;
	}

	template <typename... Ts>
	inline void add_component(Entity entity, const Ts &...components)
	{
		auto signature = world()->component_manager.template signature_from_components<Ts...>();
		(world()->component_manager.template add_component<Ts>(entity, components), ...);

		const auto old_signature = world()->entity_manager.get_signature(entity);

		world()->entity_manager.add_signature(entity, signature);
		world()->system_manager.entity_signature_changed(entity, old_signature, signature);
	}

	template <typename... Ts>
	inline Tuple<Ts &...> get_components(Entity entity)
	{
		return world()->component_manager.get_components<Ts...>(entity);
	}

	template <typename T>
	inline Option<T &> try_get_component(Entity entity)
	{
		return world()->component_manager.try_get_component<T>(entity);
	}

	template <typename T>
	inline T &get_component(Entity entity)
	{
		return world()->component_manager.get_component<T>(entity);
	}

	template <typename T>
	ErrorOr<void> try_remove_component(Entity entity)
	{
		auto res = world()->component_manager.template try_remove_component<T>(entity);
		if (res.has_error())
			return res.get_error();

		auto signature            = world()->component_manager.template signature_from_components<T>();
		const auto &old_signature = world()->entity_manager.get_signature(entity);

		world()->entity_manager.remove_signature(entity, signature);
		world()->system_manager.entity_signature_changed(entity, old_signature, signature);

		return Success;
	}

	template <typename T>
	void remove_component(Entity entity)
	{
		world()->component_manager.template remove_component<T>(entity);

		auto signature            = world()->component_manager.template signature_from_components<T>();
		const auto &old_signature = world()->entity_manager.get_signature(entity);

		world()->entity_manager.remove_signature(entity, signature);
		world()->system_manager.entity_signature_changed(entity, old_signature, signature);
	}

	template <typename... Ts>
	inline Entity create_entity(const StringView &label, const Ts &...components)
	{
		auto entity = world()->entity_manager.create_entity(label);
		if constexpr (sizeof...(Ts) != 0)
		{
			add_component<Ts...>(entity, components...);
		}
		return entity;
	}

	inline const Signature &get_signature(Entity entity) { return world()->entity_manager.get_signature(entity); }

	template <typename... Ts>
	inline bool has_component(Entity entity)
	{
		auto signature = world()->component_manager.template signature_from_components<Ts...>();
		return (get_signature(entity) & signature) == signature;
	}

	template <typename... Ts>
	requires(TypeList<Ts...>::template contains<Transform>()) inline Entity create_parented_entity(const StringView &label, Entity parent, const Ts &...components)
	{
		ASSERT(parent != INVALID_ENTITY, "Cannot create entity with invalid parent!");
		ASSERT(has_component<Transform>(parent), "Cannot create entity to parent entity without a transform!");
		auto entity = world()->entity_manager.create_entity(label, parent);
		if constexpr (sizeof...(Ts) != 0)
		{
			add_component<Ts...>(entity, components...);
		}
		return entity;
	}

	inline String *get_label(Entity entity) { return world()->entity_manager.get_label(entity); }

	inline void unparent(Entity entity) { world()->entity_manager.unparent(entity); }

	inline Option<Entity> get_parent(Entity entity) { return world()->entity_manager.get_parent(entity); }
	inline bool has_parent(Entity entity) { return get_parent(entity).has_value(); }
	inline const HashTable<Entity> &get_children(Entity entity) { return world()->entity_manager.get_children(entity); }

	inline Mat4 get_world_transform(Entity entity)
	{
		Mat4 accumulator = model_matrix(get_component<Transform>(entity));
		Entity current   = entity;
		while (true)
		{
			if let (auto parent, get_parent(current))
			{
				accumulator = model_matrix(get_component<Transform>(parent)) * accumulator;
				current     = parent;
			}
			else
			{
				return accumulator;
			}
		}
	}

	inline Mat4 get_local_transform(Mat4 world_transform, Entity entity)
	{
		Mat4 reverser  = world_transform;
		Entity current = entity;
		while (true)
		{
			if let (auto parent, get_parent(current))
			{
				reverser = glm::inverse(model_matrix(get_component<Transform>(parent))) * reverser;
				current  = parent;
			}
			else
			{
				return reverser;
			}
		}
	}

	inline void reparent_entity(Entity entity, Entity parent)
	{
		ASSERT(has_component<Transform>(parent), "Cannot reparent entity to parent entity without a transform!");
		Mat4 world_transform = get_world_transform(entity);
		world()->entity_manager.reparent(entity, parent);
		Mat4 local_transform = get_local_transform(world_transform, entity);

		auto &transform      = get_component<Transform>(entity);
		Vec3 translation;
		Vec3 rotation;
		Vec3 scale;
		Math::decompose_transform(local_transform, translation, rotation, scale);

		Vec3 deltaRotation = rotation - glm::eulerAngles(transform.rotation);
		transform.position = translation;
		transform.rotation = Quat(rotation);
		transform.scale    = scale;
	}

	inline void destroy_entity(Entity entity)
	{
		for (auto child : get_children(entity))
		{
			destroy_entity(child);
		}
		const auto &signature = get_signature(entity);
		world()->system_manager.entity_destroyed(entity, signature);
		world()->component_manager.destroy_entity(entity);
		world()->entity_manager.destroy_entity(entity);
	}

	inline void register_system(void *component, const Signature &signature, System::EntityCreated entity_created = nullptr, System::EntityDestroyed entity_destroyed = nullptr)
	{
		auto system = new_system(component, signature, entity_created, entity_destroyed);
		world()->system_manager.register_system(system);
	}

	template <typename... Ts>
	inline constexpr Signature signature_from_components()
	{
		return world()->component_manager.signature_from_components<Ts...>();
	}

	template <typename... Ts>
	World::IteratorContainer<Ts...> get_entities()
	{
		return World::IteratorContainer<Ts...>(world());
	}

	inline bool entity_exists(Entity entity) { return world()->entity_manager.entity_exists(entity); }
} // namespace Vultr
