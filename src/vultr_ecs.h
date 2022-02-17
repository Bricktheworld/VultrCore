#pragma once
#include "ecs/world.h"
#include "core/components/components.h"

namespace Vultr
{
	using World = WorldT<Mesh, Transform, Material, Camera>;

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
	inline Entity create_entity(const Ts &...components)
	{
		auto entity = world()->entity_manager.create_entity();
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

	inline void destroy_entity(Entity entity)
	{
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
} // namespace Vultr
