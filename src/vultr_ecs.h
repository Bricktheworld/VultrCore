#pragma once
#include "ecs/world.h"
#include "core/components/components.h"
#include <math/decompose_transform.h>

namespace Vultr
{
	using World = WorldT<Mesh, Transform, Material, Camera, DirectionalLight, PointLight>;

	inline World *world()
	{
		ASSERT(g_game_memory != nullptr && g_game_memory->world != nullptr, "Game memory not properly initialized!");
		return static_cast<World *>(g_game_memory->world);
	}

	template <typename T>
	inline ErrorOr<void> world_try_add_component(World *w, Entity entity, const T &component)
	{
		auto res = w->component_manager.template try_add_component(entity, component);
		if (res.has_error())
			return res.get_error();

		auto signature            = w->component_manager.template signature_from_components<T>();
		const auto &old_signature = w->entity_manager.get_signature(entity);

		w->entity_manager.add_signature(entity, signature);
		//		system_manager()->entity_signature_changed(entity, old_signature, signature);
		return Success;
	}

	template <typename T>
	inline ErrorOr<void> try_add_component(World *w, Entity entity, const T &component)
	{
		return world_try_add_component(world(), entity, component);
	}

	template <typename... Ts>
	inline void world_add_component(World *w, Entity entity, const Ts &...components)
	{
		auto signature = w->component_manager.template signature_from_components<Ts...>();
		(w->component_manager.template add_component<Ts>(entity, components), ...);

		const auto old_signature = w->entity_manager.get_signature(entity);

		w->entity_manager.add_signature(entity, signature);
		//		system_manager()->entity_signature_changed(entity, old_signature, signature);
	}

	template <typename... Ts>
	inline void add_component(Entity entity, const Ts &...components)
	{
		world_add_component(world(), entity, components...);
	}

	template <typename... Ts>
	inline Tuple<Ts &...> world_get_components(World *w, Entity entity)
	{
		return w->component_manager.get_components<Ts...>(entity);
	}

	template <typename... Ts>
	inline Tuple<Ts &...> get_components(Entity entity)
	{
		return world_get_components<Ts...>(entity);
	}

	template <typename T>
	inline Option<T &> world_try_get_component(World *w, Entity entity)
	{
		return w->component_manager.try_get_component<T>(entity);
	}

	template <typename T>
	inline Option<T &> try_get_component(Entity entity)
	{
		return world_try_get_component<T>(world(), entity);
	}

	template <typename T>
	inline T &world_get_component(World *w, Entity entity)
	{
		return w->component_manager.get_component<T>(entity);
	}

	template <typename T>
	inline T &get_component(Entity entity)
	{
		return world_get_component<T>(world(), entity);
	}

	template <typename T>
	ErrorOr<void> world_try_remove_component(World *w, Entity entity)
	{
		auto res = w->component_manager.template try_remove_component<T>(entity);
		if (res.has_error())
			return res.get_error();

		auto signature            = w->component_manager.template signature_from_components<T>();
		const auto &old_signature = w->entity_manager.get_signature(entity);

		w->entity_manager.remove_signature(entity, signature);
		//		system_manager()->entity_signature_changed(entity, old_signature, signature);

		return Success;
	}

	template <typename T>
	ErrorOr<void> try_remove_component(Entity entity)
	{
		return world_try_remove_component<T>(world(), entity);
	}

	template <typename T>
	void world_remove_component(World *w, Entity entity)
	{
		w->component_manager.template remove_component<T>(entity);

		auto signature            = w->component_manager.template signature_from_components<T>();
		const auto &old_signature = w->entity_manager.get_signature(entity);

		w->entity_manager.remove_signature(entity, signature);
		//		system_manager()->entity_signature_changed(entity, old_signature, signature);
	}

	template <typename T>
	void remove_component(Entity entity)
	{
		world_remove_component<T>(world(), entity);
	}

	template <typename... Ts>
	inline Entity world_create_entity(World *w, const StringView &label, const Ts &...components)
	{
		auto entity = world()->entity_manager.create_entity(label, Platform::generate_uuid());
		if constexpr (sizeof...(Ts) != 0)
		{
			add_component<Ts...>(entity, components...);
		}
		return entity;
	}

	template <typename... Ts>
	inline Entity create_entity(const StringView &label, const Ts &...components)
	{
		return world_create_entity(world(), label, components...);
	}

	inline const Signature &world_get_signature(World *w, Entity entity) { return w->entity_manager.get_signature(entity); }
	inline const Signature &get_signature(Entity entity) { return world_get_signature(world(), entity); }

	template <typename... Ts>
	inline bool world_has_component(World *w, Entity entity)
	{
		auto signature = w->component_manager.template signature_from_components<Ts...>();
		return (get_signature(entity) & signature) == signature;
	}

	template <typename... Ts>
	inline bool has_component(Entity entity)
	{
		return world_has_component<Ts...>(world(), entity);
	}

	template <typename... Ts>
		requires(TypeList<Ts...>::template contains<Transform>())
	inline Entity world_create_parented_entity(World *w, const StringView &label, Entity parent, const Ts &...components)
	{
		ASSERT(parent != INVALID_ENTITY, "Cannot create entity with invalid parent!");
		ASSERT(has_component<Transform>(parent), "Cannot create entity to parent entity without a transform!");
		auto entity = w->entity_manager.create_entity(label, Platform::generate_uuid(), parent);
		if constexpr (sizeof...(Ts) != 0)
		{
			add_component<Ts...>(entity, components...);
		}
		return entity;
	}

	template <typename... Ts>
		requires(TypeList<Ts...>::template contains<Transform>())
	inline Entity create_parented_entity(const StringView &label, Entity parent, const Ts &...components)
	{
		return world_create_parented_entity(world(), label, parent, components...);
	}

	//	 inline Entity en(World *w, const StringView &label, Entity parent)
	//	{
	//		ASSERT(parent != INVALID_ENTITY, "Cannot create entity with invalid parent!");
	//		ASSERT(has_component<Transform>(parent), "Cannot create entity to parent entity without a transform!");
	//		auto entity = w->entity_manager.create_entity(label, parent);
	//		if constexpr (sizeof...(Ts) != 0)
	//		{
	//			add_component<Ts...>(entity, components...);
	//		}
	//		return entity;
	//	}

	inline String *world_get_label(World *w, Entity entity) { return w->entity_manager.get_label(entity); }
	inline String *get_label(Entity entity) { return world()->entity_manager.get_label(entity); }

	inline UUID world_get_uuid(World *w, Entity entity) { return w->entity_manager.get_uuid(entity); }
	inline UUID get_uuid(World *w, Entity entity) { return world()->entity_manager.get_uuid(entity); }

	inline Entity world_get_entity(World *w, const UUID &uuid) { return w->entity_manager.get_entity(uuid); }
	inline Entity get_entity(const UUID &uuid) { return world()->entity_manager.get_entity(uuid); }

	inline Option<Entity> world_get_parent(World *w, Entity entity) { return w->entity_manager.get_parent(entity); }
	inline Option<Entity> get_parent(Entity entity) { return world_get_parent(world(), entity); }

	inline bool world_has_parent(World *w, Entity entity) { return world_get_parent(w, entity).has_value(); }
	inline bool has_parent(Entity entity) { return world_has_parent(world(), entity); }

	inline const HashTable<Entity> &world_get_children(World *w, Entity entity) { return w->entity_manager.get_children(entity); }
	inline const HashTable<Entity> &get_children(Entity entity) { return world_get_children(world(), entity); }

	inline Mat4 world_get_world_transform(World *w, Entity entity)
	{
		Mat4 accumulator = model_matrix(world_get_component<Transform>(w, entity));
		Entity current   = entity;
		while (true)
		{
			if let (auto parent, world_get_parent(w, current))
			{
				accumulator = model_matrix(world_get_component<Transform>(w, parent)) * accumulator;
				current     = parent;
			}
			else
			{
				return accumulator;
			}
		}
	}

	inline Mat4 get_world_transform(Entity entity) { return world_get_world_transform(world(), entity); }

	inline Mat4 world_get_local_transform(World *w, Mat4 world_transform, Entity entity)
	{
		if let (auto parent, world_get_parent(w, entity))
		{
			return glm::inverse(world_get_world_transform(w, parent)) * world_transform;
		}
		else
		{
			return world_transform;
		}
	}

	inline Mat4 get_local_transform(Mat4 world_transform, Entity entity) { return world_get_local_transform(world(), world_transform, entity); }

	inline void world_reparent_entity(World *w, Entity entity, Entity parent)
	{
		ASSERT(world_has_component<Transform>(w, parent), "Cannot reparent entity to parent entity without a transform!");
		Mat4 world_transform = world_get_world_transform(w, entity);
		w->entity_manager.reparent(entity, parent);
		Mat4 local_transform = world_get_local_transform(w, world_transform, entity);

		auto &transform      = world_get_component<Transform>(w, entity);
		Math::decompose_transform(local_transform, &transform.position, &transform.rotation, &transform.scale);
	}

	inline void reparent_entity(Entity entity, Entity parent) { world_reparent_entity(world(), entity, parent); }

	inline void world_unparent(World *w, Entity entity)
	{
		if (world_has_component<Transform>(w, entity))
		{
			Mat4 world_transform = world_get_world_transform(w, entity);
			auto &transform      = world_get_component<Transform>(w, entity);

			Math::decompose_transform(world_transform, &transform.position, &transform.rotation, &transform.scale);
		}
		w->entity_manager.unparent(entity);
	}
	inline void unparent(Entity entity) { world_unparent(world(), entity); }

	inline void world_destroy_entity(World *w, Entity entity)
	{
		for (auto child : get_children(entity))
		{
			world_destroy_entity(w, child);
		}
		const auto &signature = world_get_signature(w, entity);
		//		system_manager()->entity_destroyed(entity, signature);
		w->component_manager.destroy_entity(entity);
		w->entity_manager.destroy_entity(entity);
	}

	inline void destroy_entity(Entity entity) { world_destroy_entity(world(), entity); }

	//	inline void register_system(void *component, const Signature &signature, System::EntityCreated entity_created = nullptr, System::EntityDestroyed entity_destroyed = nullptr)
	//	{
	//		auto system = new_system(component, signature, entity_created, entity_destroyed);
	//		//		system_manager()->register_system(system);
	//	}

	template <typename... Ts>
	inline Signature world_signature_from_components(World *w)
	{
		return w->component_manager.signature_from_components<Ts...>();
	}

	template <typename... Ts>
	inline Signature signature_from_components()
	{
		return world_signature_from_components<Ts...>(world());
	}

	template <typename... Ts>
	World::IteratorContainer<Ts...> world_get_entities(World *w)
	{
		return World::IteratorContainer<Ts...>(w);
	}

	template <typename... Ts>
	World::IteratorContainer<Ts...> get_entities()
	{
		return world_get_entities<Ts...>(world());
	}

	inline bool world_entity_exists(World *w, Entity entity) { return w->entity_manager.entity_exists(entity); }
	inline bool entity_exists(Entity entity) { return world_entity_exists(world(), entity); }
} // namespace Vultr
