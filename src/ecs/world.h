#pragma once
#include <types/tuple.h>
#include <types/queue.h>
#include <types/hashmap.h>
#include "entity.h"
#include "component.h"
#include "system.h"

namespace Vultr
{
	struct EntityManager
	{
		Queue<Entity, MAX_ENTITIES> m_queue{};
		Hashmap<Entity, Signature> m_living_entities{};

		EntityManager()
		{
			for (u32 entity = 0; entity < MAX_ENTITIES; entity++)
			{
				m_queue.push(entity);
			}
		}

		Entity create_entity()
		{
			Entity new_entity = m_queue.pop();
			m_living_entities.set(new_entity, Signature());
			return new_entity;
		}

		void set_signature(Entity entity, const Signature &signature) { m_living_entities.set(entity, signature); }
		const Signature &get_signature(Entity entity) { return m_living_entities.get(entity); }

		void destroy_entity(Entity entity)
		{
			ASSERT(m_living_entities.remove(entity), "Entity has already been destroyed!");
			m_queue.push(entity);
		}
	};

	template <typename T>
	struct ComponentArray
	{
		ComponentArray() { m_array = v_alloc<T>(MAX_ENTITIES); }
		ErrorOr<void> add_entity(Entity entity, const T &component)
		{
			if (m_entity_to_index.contains(entity))
				return Error("Component added to the same entity more than once!");

			auto new_index = m_size;
			m_entity_to_index.set(entity, new_index);
			m_index_to_entity.set(new_index, entity);
			storage()[new_index] = T(component);

			m_size++;

			return None;
		}

		Option<T &> get_component(Entity entity)
		{
			if let (auto index, m_entity_to_index.try_get(entity))
				return storage()[index];
			else
				return None;
		}

		bool has_component(Entity entity) { return m_entity_to_index.contains(entity); }

		ErrorOr<void> remove_entity(Entity entity)
		{
			if let (size_t removed_index, m_entity_to_index.try_get(entity))
			{
				size_t last_index      = m_size - 1;

				m_array[removed_index] = m_array[last_index];

				Entity last_entity     = m_index_to_entity.get(last_index);
				m_entity_to_index.set(last_entity, removed_index);
				m_index_to_entity.set(removed_index, last_entity);

				m_entity_to_index.remove(entity);
				m_index_to_entity.remove(last_index);

				m_size--;
			}
			else
			{
				return Error("Attempting to remove nonexistent component!");
			}
		}

		T *storage() { return static_cast<T *>(m_array); }

		T *m_array = nullptr;
		Hashmap<Entity, size_t> m_entity_to_index{};
		Hashmap<size_t, Entity> m_index_to_entity{};
		size_t m_size = 0;
	};

	template <typename T>
	void init_component_array(void **out)
	{
		*out = v_alloc<ComponentArray<T>>();
	}

	template <typename... Component>
	struct ComponentManager
	{
		using Types = TypeList<Component...>;
		void *component_arrays[MAX_COMPONENTS]{};
		ComponentManager() { m_init(typename SequenceImpl<sizeof...(Component)>::type()); }

		template <typename T>
		size_t get_component_index()
		{
			static_assert(Types::template contains<T>(), "Component is not a part of the component manager!");
			return Types::template index_of<T>();
		}

		template <typename T>
		ComponentArray<T> *get_component_array()
		{
			return static_cast<ComponentArray<T> *>(component_arrays[get_component_index<T>()]);
		}

		template <typename T>
		ErrorOr<void> try_add_component(Entity entity, const T &component)
		{
			return get_component_array<T>()->add_entity(entity, component);
		}

		template <typename T>
		void add_component(Entity entity, const T &component)
		{
			auto res = try_add_component<T>(entity);
			ASSERT(res, res.get_error().message.c_str());
		}

		template <typename T>
		Option<T &> try_get_component(Entity entity)
		{
			return get_component_array<T>()->get_component(entity);
		}

		template <typename T>
		T &get_component(Entity entity)
		{
			auto res = try_get_component<T>(entity);
			ASSERT(res, "Component does not exist!");
			return res.value();
		}

		template <typename... Ts>
		Tuple<Ts &...> get_components(Entity entity)
		{
			return Tuple<Ts &...>(get_component<Ts>(entity)...);
		}

	  private:
		template <size_t... S>
		void m_init(Sequence<S...>)
		{
			(init_component_array<Component>(&component_arrays[S]), ...);
		}
	};

	struct SystemManager
	{
		Vector<System> systems{};

		void register_system(const System &system) { systems.push_back(system); }
	};

	template <typename... Component>
	struct World
	{
		EntityManager entity_manager{};
		ComponentManager<Component...> component_manager{};
		SystemManager system_manager{};

		template <typename... Ts>
		struct IteratorContainer
		{
			struct EntityIterator
			{
				EntityIterator(World *world, Entity entity) : m_world(world), m_entity(entity) {}

				Tuple<Ts &...> operator*() { return m_world->component_manager.template get_components<Ts...>(m_entity); }
				bool operator==(const EntityIterator &other) const { return m_entity == other.m_entity; }

				EntityIterator &operator++()
				{
					skip_to_next();
					return *this;
				}

				EntityIterator operator++(int)
				{
					EntityIterator cpy = *this;
					skip_to_next();
					return cpy;
				}

				void skip_to_next() {}

				Entity m_entity;
				World *m_world;
			};

			static EntityIterator begin() {}
			static EntityIterator end() {}
		};

		template <typename... Ts>
	};

} // namespace Vultr