#pragma once
#include <types/tuple.h>
#include <types/queue.h>
#include "entity.h"
#include "component.h"

namespace Vultr
{
	struct EntityManager
	{
		Queue<Entity, MAX_ENTITIES> queue;
		EntityManager()
		{
			for (u32 entity = 0; entity < MAX_ENTITIES; entity++)
			{
				queue.push(entity);
			}
		}

		Entity create_entity() { return queue.pop(); }
		void destroy_entity(Entity entity) {}
	};

	struct ComponentArray
	{
		typedef ErrorOr<void> (*AddEntity)(ComponentArray *array, Entity entity);
		typedef Option<void *> (*GetComponent)(ComponentArray *array, Entity entity);
		typedef ErrorOr<void> (*RemoveEntity)(ComponentArray *array, Entity entity);

		void *m_array = nullptr;
		AddEntity m_add_entity{};
		GetComponent m_get_component{};
		RemoveEntity m_remove_entity{};
	};

	template <typename T>
	ErrorOr<void> component_array_add_entity(ComponentArray *array, Entity entity)
	{
		return None;
	}

	template <typename T>
	Option<void *> component_array_get_component(ComponentArray *array, Entity entity)
	{
		return None;
	}

	template <typename T>
	ErrorOr<void> component_array_remove_entity(ComponentArray *array, Entity entity)
	{
		return None;
	}

	template <typename T>
	void fill_component_array(ComponentArray *array)
	{
		array->m_array         = v_alloc<T>(MAX_ENTITIES);
		array->m_add_entity    = component_array_add_entity<T>;
		array->m_get_component = component_array_get_component<T>;
		array->m_remove_entity = component_array_remove_entity<T>;
	}

	template <typename... Component>
	struct ComponentManager
	{
		using Types = TypeList<Component...>;
		ComponentArray component_arrays[MAX_COMPONENTS]{};
		ComponentManager() { init_impl(typename SequenceImpl<sizeof...(Component)>::type()); }

		template <typename T>
		size_t get_component_index()
		{
			static_assert(Types::template contains<T>(), "Component is not a part of the component manager!");
			return Types::template index_of<T>();
		}

	  private:
		template <size_t... S>
		void init_impl(Sequence<S...>)
		{
			(fill_component_array<Component>(&component_arrays[S]), ...);
		}
	};

	struct SystemManager
	{
	};

	template <typename... Component>
	struct World
	{
		EntityManager entity_manager{};
		ComponentManager<Component...> component_manager{};
		SystemManager system_manager{};
	};
} // namespace Vultr