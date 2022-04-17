#pragma once
#include <types/tuple.h>
#include <types/queue.h>
#include <types/hashmap.h>
#include <types/vector.h>
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
			for (u32 entity = 1; entity < MAX_ENTITIES; entity++)
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

		void add_signature(Entity entity, const Signature &signature) { m_living_entities.set(entity, get_signature(entity) | signature); }
		void remove_signature(Entity entity, const Signature &signature) { m_living_entities.set(entity, get_signature(entity) & (~signature)); }
		const Signature &get_signature(Entity entity) { return m_living_entities.get(entity); }
		bool entity_exists(Entity entity) { return m_living_entities.contains(entity); }

		void destroy_entity(Entity entity)
		{
			auto res = m_living_entities.remove(entity);
			ASSERT(res, "Entity has already been destroyed!");
			m_queue.push(entity);
		}
	};

	struct ComponentInformation
	{
		StringView component_name{};
		Vector<ComponentMember> component_members{};
	};

	struct IComponentArray
	{
		typedef ErrorOr<void> (*TryRemoveEntity)(IComponentArray *arr, Entity entity);
		typedef void (*GetComponentInformation)(IComponentArray *arr, Entity entity, Vector<ComponentInformation> &info);
		TryRemoveEntity m_try_remove_entity;
		GetComponentInformation m_get_component_information;
		explicit IComponentArray(TryRemoveEntity try_remove_entity, GetComponentInformation get_component_information) : m_try_remove_entity(try_remove_entity), m_get_component_information(get_component_information)
		{
		}

		ErrorOr<void> try_remove_entity(Entity entity) { return m_try_remove_entity(this, entity); }
		void get_component_information(Entity entity, Vector<ComponentInformation> &info) { return m_get_component_information(this, entity, info); }
	};

	template <typename T>
	struct ComponentArray : IComponentArray
	{
		typedef Vector<ComponentMember> (*MemberAPI)(T *);
		ComponentArray() : IComponentArray(try_remove_entity_impl, get_component_information_impl), m_member_api(ComponentTraits<T>::members), m_component_name(ComponentTraits<T>::type_name())
		{
			m_array = v_alloc<T>(MAX_ENTITIES);
		}
		ErrorOr<void> add_entity(Entity entity, const T &component)
		{
			if (m_entity_to_index.contains(entity))
				return Error("DefaultComponent added to the same entity more than once!");

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

		static ErrorOr<void> try_remove_entity_impl(IComponentArray *p_arr, Entity entity)
		{
			auto *arr = static_cast<ComponentArray<T> *>(p_arr);
			if let (size_t removed_index, arr->m_entity_to_index.try_get(entity))
			{
				size_t last_index = arr->m_size - 1;

				arr->m_array[removed_index].~T();
				Utils::move(arr->m_array + removed_index, arr->m_array + last_index, 1);

				Entity last_entity = arr->m_index_to_entity.get(last_index);
				arr->m_entity_to_index.set(last_entity, removed_index);
				arr->m_index_to_entity.set(removed_index, last_entity);

				arr->m_entity_to_index.remove(entity);
				arr->m_index_to_entity.remove(last_index);

				arr->m_size--;
				return None;
			}
			else
			{
				return Error("Attempting to remove nonexistent component!");
			}
		}

		static void get_component_information_impl(IComponentArray *p_arr, Entity entity, Vector<ComponentInformation> &component_members)
		{
			auto *arr = static_cast<ComponentArray<T> *>(p_arr);
			if (!arr->has_component(entity))
				return;

			component_members.push_back(ComponentInformation{
				.component_name    = arr->m_component_name,
				.component_members = arr->m_member_api(&arr->get_component(entity).value()),
			});
		}

		T *storage() { return static_cast<T *>(m_array); }

		T *m_array                  = nullptr;
		MemberAPI m_member_api      = nullptr;
		StringView m_component_name = "";
		Hashmap<Entity, size_t> m_entity_to_index{};
		Hashmap<size_t, Entity> m_index_to_entity{};
		size_t m_size = 0;
	};

	template <typename T>
	void init_component_array(IComponentArray **out)
	{
		*out = v_alloc<ComponentArray<T>>();
	}

	template <typename... DefaultComponent>
	struct ComponentManager
	{
		using Types = TypeList<DefaultComponent...>;
		Vector<IComponentArray *, sizeof...(DefaultComponent)> component_arrays{};
		Hashmap<u32, size_t> type_to_index;

		ComponentManager()
		{
			component_arrays.resize(sizeof...(DefaultComponent));
			m_init(typename SequenceImpl<sizeof...(DefaultComponent)>::type());
		}

		template <typename T>
		void register_component()
		{
			constexpr u32 type_id = ComponentTraits<T>::component_id();
			ASSERT(!type_to_index.contains(type_id), "Component already registered!");
			size_t index = component_arrays.size();
			component_arrays.push_back(v_alloc<ComponentArray<T>>());
			type_to_index.set(type_id, index);
		}

		template <typename T>
		size_t get_component_index()
		{
			if constexpr (Types::template contains<T>())
				return Types::template index_of<T>();
			else
				type_to_index.get(ReflTraits<T>::type_id());
		}

		template <typename... Ts>
		constexpr Signature signature_from_components()
		{
			Signature signature{};
			(signature.set(get_component_index<Ts>(), true), ...);
			return signature;
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
			auto res = try_add_component<T>(entity, component);
			ASSERT(res, "%s", res.get_error().message.c_str());
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

		template <typename T>
		ErrorOr<void> try_remove_component(Entity entity)
		{
			return get_component_array<T>()->remove_entity(entity);
		}

		template <typename T>
		void remove_component(Entity entity)
		{
			auto res = try_remove_component<T>(entity);
			ASSERT(res, "%s", res.get_error().message.c_str());
		}

		void destroy_entity(Entity entity)
		{
			(get_component_array<DefaultComponent>()->try_remove_entity(entity), ...);
			for (auto &[type_id, index] : type_to_index)
			{
				component_arrays[index]->try_remove_entity(entity);
			}
		}

		Vector<ComponentInformation> get_component_information(Entity entity)
		{
			Vector<ComponentInformation> information{};
			(get_component_array<DefaultComponent>()->get_component_information(entity, information), ...);
			for (auto &[type_id, index] : type_to_index)
			{
				component_arrays[index]->get_component_information(entity, information);
			}
			return information;
		}

	  private:
		template <size_t... S>
		void m_init(Sequence<S...>)
		{
			(init_component_array<DefaultComponent>(&component_arrays[S]), ...);
		}
	};

	struct SystemManager
	{
		Vector<System> systems{};

		void register_system(const System &system) { systems.push_back(system); }
		void entity_created(Entity entity, const Signature &signature)
		{
			for (auto &system : systems)
			{
				if ((system.component_signature & signature) == system.component_signature)
				{
					system.m_created(system.component, entity);
				}
			}
		}
		void entity_destroyed(Entity entity, const Signature &signature)
		{
			for (auto &system : systems)
			{
				if ((system.component_signature & signature) == system.component_signature)
				{
					system.m_destroyed(system.component, entity);
				}
			}
		}
		void entity_signature_changed(Entity entity, const Signature &old_signature, const Signature &new_signature)
		{
			ASSERT(old_signature != new_signature, "Entity signature has not actually changed!");

			for (auto &system : systems)
			{
				if ((system.component_signature & new_signature) == system.component_signature)
				{
					system.m_created(system.component, entity);
				}
				else if ((system.component_signature & old_signature) == system.component_signature)
				{
					system.m_destroyed(system.component, entity);
				}
			}
		}
	};

	template <typename... Component>
	struct WorldT
	{
		EntityManager entity_manager{};
		ComponentManager<Component...> component_manager{};
		SystemManager system_manager{};

		template <typename... Ts>
		struct IteratorContainer
		{
			explicit IteratorContainer(WorldT *world) : m_world(world) {}
			struct EntityIterator
			{
				EntityIterator(WorldT *world, const Hashmap<Entity, Signature>::HIterator &iterator)
					: m_world(world), m_iterator(iterator), m_signature(world->component_manager.template signature_from_components<Ts...>())
				{
					if (!matches_signature())
						skip_to_next();
				}

				Tuple<Entity, Ts &...> operator*() { return Tuple<Entity, Ts &...>(m_iterator->key, m_world->component_manager.template get_component<Ts>(m_iterator->key)...); }
				bool operator==(const EntityIterator &other) const { return m_iterator == other.m_iterator; }

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

				void skip_to_next()
				{
					if (is_end())
						return;
					while (true)
					{
						m_iterator++;
						if (is_end() || matches_signature())
							return;
					}
				}

				bool matches_signature() { return !is_end() && (m_iterator->value & m_signature) == m_signature; }
				bool is_end() { return m_iterator == m_world->entity_manager.m_living_entities.end(); }

				Hashmap<Entity, Signature>::HIterator m_iterator;
				Signature m_signature{};
				WorldT *m_world = nullptr;
			};

			EntityIterator begin() { return EntityIterator(m_world, m_world->entity_manager.m_living_entities.begin()); }
			EntityIterator end() { return EntityIterator(m_world, m_world->entity_manager.m_living_entities.end()); }

			WorldT *m_world = nullptr;
		};
	};

} // namespace Vultr
