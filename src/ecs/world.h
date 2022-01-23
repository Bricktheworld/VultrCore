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
		void destroy_entity(Entity entity)
		{
			ASSERT(!queue.contains(entity), "Entity has already been destroyed!");
			queue.push(entity);
		}
	};

	struct ComponentArray
	{
		typedef ErrorOr<void> (*AddEntity)(ComponentArray *array, Entity entity);
		typedef void *(*GetComponent)(ComponentArray *array, Entity entity);
		typedef ErrorOr<void> (*RemoveEntity)(ComponentArray *array, Entity entity);

		void *m_array = nullptr;

		ErrorOr<void> add_entity(Entity entity) { return m_add_entity(this, entity); }
		void *get_component(Entity entity) { return m_get_component(this, entity); }
		ErrorOr<void> remove_entity(Entity entity) { return m_remove_entity(this, entity); }

		AddEntity m_add_entity{};
		GetComponent m_get_component{};
		RemoveEntity m_remove_entity{};
	};

	template <typename T>
	T *component_array_get_array(ComponentArray *array)
	{
		return reinterpret_cast<T *>(array->m_array);
	}

	template <typename T>
	ErrorOr<void> component_array_add_entity(ComponentArray *array, Entity entity)
	{
		return None;
	}

	template <typename T>
	void *component_array_get_component(ComponentArray *array, Entity entity)
	{
		return &component_array_get_array<T>(array)[entity];
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

		template <typename T>
		ComponentArray *get_component_array()
		{
			return &component_arrays[get_component_index<T>()];
		}

		template <typename T>
		T *try_get_component(Entity entity)
		{
			return static_cast<T *>(get_component_array<T>()->get_component(entity));
		}

		template <typename T>
		T &get_component(Entity entity)
		{
			auto res = try_get_component<T>(entity);
			ASSERT(res != nullptr, "Component does not exist!");
			return *res;
		}

		template <typename... Ts>
		Tuple<Ts &...> get_components(Entity entity)
		{
			return Tuple<Ts &...>(get_component<Ts>(entity)...);
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