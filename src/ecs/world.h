#pragma once
#include <types/tuple.h>
#include <types/queue.h>
#include <types/hashmap.h>
#include <types/vector.h>
#include <types/string.h>
#include "entity.h"
#include "component.h"
#include "system.h"

namespace Vultr
{
	struct EntityInfo
	{
		String label{};
		Signature signature{};
		Entity parent = INVALID_ENTITY;
		HashTable<Entity> children{};
	};

	struct EntityManager
	{
		Queue<Entity, MAX_ENTITIES> m_queue{};
		Hashmap<Entity, EntityInfo> m_living_entities{};

		EntityManager()
		{
			for (u32 entity = 1; entity < MAX_ENTITIES; entity++)
			{
				m_queue.push(entity);
			}
		}

		Entity create_entity(const StringView &label, Entity parent = INVALID_ENTITY)
		{
			ASSERT(parent == INVALID_ENTITY || entity_exists(parent), "Cannot re-parent to non-existent parent entity!");
			Entity new_entity = m_queue.pop();
			m_living_entities.set(new_entity, {.label = String(label), .parent = parent});
			if (parent != INVALID_ENTITY)
				m_living_entities.get(parent).children.set<Entity>(new_entity);
			return new_entity;
		}

		void add_signature(Entity entity, const Signature &signature) { m_living_entities.get(entity).signature |= signature; }
		void remove_signature(Entity entity, const Signature &signature) { m_living_entities.get(entity).signature &= (~signature); }
		const Signature &get_signature(Entity entity) { return m_living_entities.get(entity).signature; }
		String *get_label(Entity entity) { return &m_living_entities.get(entity).label; }

		Option<Entity> get_parent(Entity entity)
		{
			ASSERT(entity_exists(entity), "Cannot get parent of non-existent entity!");

			Entity parent = m_living_entities.get(entity).parent;

			if (parent == INVALID_ENTITY)
				return None;

			return parent;
		}

		void reparent(Entity entity, Entity new_parent)
		{
			ASSERT(entity_exists(entity), "Cannot re-parent non-existent entity!");
			ASSERT(entity_exists(new_parent), "Cannot re-parent to non-existent parent entity!");

			auto current_parent                  = m_living_entities.get(entity).parent;
			m_living_entities.get(entity).parent = new_parent;

			if (current_parent != INVALID_ENTITY)
				m_living_entities.get(current_parent).children.remove(entity);

			m_living_entities.get(new_parent).children.set<Entity>(entity);
		}

		void unparent(Entity entity)
		{
			ASSERT(entity_exists(entity), "Cannot un-parent non-existent entity!");
			auto current_parent = m_living_entities.get(entity).parent;
			if (current_parent != INVALID_ENTITY)
			{
				m_living_entities.get(entity).parent = INVALID_ENTITY;
				m_living_entities.get(current_parent).children.remove<Entity>(entity);
			}
		}

		bool entity_exists(Entity entity) { return m_living_entities.contains(entity); }

		const HashTable<Entity> &get_children(Entity entity) { return m_living_entities.get(entity).children; }

		void destroy_entity(Entity entity)
		{
			auto parent = m_living_entities.get(entity).parent;
			if (parent != INVALID_ENTITY)
			{
				m_living_entities.get(parent).children.remove(entity);
			}
			auto res = m_living_entities.remove(entity);
			ASSERT(res, "Entity has already been destroyed!");
			m_queue.push(entity);
		}
	};

	struct IComponentArray
	{
		typedef void (*GetRttiApi)(IComponentArray *, Entity, Vector<Tuple<Type, void *>> *);
		typedef void (*GetEditorRttiApi)(IComponentArray *, Entity, Vector<EditorType> *);
		typedef ErrorOr<void> (*TryRemoveEntityApi)(IComponentArray *arr, Entity entity);
		typedef void (*DestroyApi)(IComponentArray *arr);
		explicit IComponentArray(TryRemoveEntityApi try_remove_entity, DestroyApi destroy_api, const Type &rtti_type, GetRttiApi get_rtti, GetEditorRttiApi get_editor_rtti)
			: m_try_remove_entity(try_remove_entity), m_destroy(destroy_api), rtti_type(rtti_type), m_get_rtti(get_rtti), m_get_editor_rtti(get_editor_rtti)
		{
		}

		void destroy() { m_destroy(this); }
		void get_rtti(Entity entity, Vector<Tuple<Type, void *>> *rtti) { m_get_rtti(this, entity, rtti); }
		void get_editor_rtti(Entity entity, Vector<EditorType> *rtti) { m_get_editor_rtti(this, entity, rtti); }

		void remove_entity(Entity entity) { CHECK(try_remove_entity(entity)); }
		ErrorOr<void> try_remove_entity(Entity entity) { return m_try_remove_entity(this, entity); }
		TryRemoveEntityApi m_try_remove_entity = nullptr;
		DestroyApi m_destroy                   = nullptr;
		GetRttiApi m_get_rtti                  = nullptr;
		GetEditorRttiApi m_get_editor_rtti     = nullptr;

		Type rtti_type{};
		Hashmap<Entity, size_t> m_entity_to_index{};
		Hashmap<size_t, Entity> m_index_to_entity{};
		size_t m_size = 0;
		void *m_array = nullptr;
	};

	template <typename T>
	struct ComponentArray : IComponentArray
	{
		ComponentArray() : IComponentArray(try_remove_entity_impl, destroy_impl, get_type<T>, get_rtti_impl, get_editor_rtti_impl) { m_array = v_alloc<T>(MAX_ENTITIES); }
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

		static void destroy_impl(IComponentArray *p_arr)
		{
			auto *arr = static_cast<ComponentArray<T> *>(p_arr);
			for (size_t i = 0; i < arr->m_size; i++)
				arr->storage()[i].~T();

			arr->m_entity_to_index.clear();
			arr->m_index_to_entity.clear();
			arr->m_size = 0;
		}

		static ErrorOr<void> try_remove_entity_impl(IComponentArray *p_arr, Entity entity)
		{
			auto *arr = static_cast<ComponentArray<T> *>(p_arr);
			if let (size_t removed_index, arr->m_entity_to_index.try_get(entity))
			{
				size_t last_index = arr->m_size - 1;

				arr->storage()[removed_index].~T();
				Utils::move(arr->storage() + removed_index, arr->storage() + last_index, 1);

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

		static void get_rtti_impl(IComponentArray *p_arr, Entity entity, Vector<Tuple<Type, void *>> *rtti)
		{
			auto *arr = static_cast<ComponentArray<T> *>(p_arr);
			if (!arr->has_component(entity))
				return;

			rtti->push_back(Tuple<Type, void *>(get_type<T>, &arr->get_component(entity).value()));
		}

		static void get_editor_rtti_impl(IComponentArray *p_arr, Entity entity, Vector<EditorType> *editor_rtti)
		{
			auto *arr = static_cast<ComponentArray<T> *>(p_arr);
			if (!arr->has_component(entity))
				return;

			auto *component = &arr->get_component(entity).value();
			editor_rtti->push_back(get_editor_type(component));
		}

		T *storage() { return static_cast<T *>(m_array); }
	};

	template <typename T>
	void init_component_array(IComponentArray **out, Hashmap<u32, size_t> *type_to_index, u32 index)
	{
		*out = v_alloc<ComponentArray<T>>();
		type_to_index->set(get_type<T>.id, index);
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
			constexpr u32 type_id = get_type<T>.id;
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
				type_to_index.get(get_type<T>.id);
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

		Vector<Tuple<Type, u32>> get_component_rtti()
		{
			Vector<Tuple<Type, u32>> types{};
			for (auto &[type_id, index] : type_to_index)
			{
				types.push_back(Tuple<Type, u32>(component_arrays[index]->rtti_type, index));
			}
			return types;
		}

		Vector<Tuple<Type, void *>> get_component_rtti(Entity entity)
		{
			Vector<Tuple<Type, void *>> types{};
			for (auto &[type_id, index] : type_to_index)
			{
				component_arrays[index]->get_rtti(entity, &types);
			}
			return types;
		}

		Vector<EditorType> get_component_editor_rtti(Entity entity)
		{
			Vector<EditorType> editor_types{};
			for (auto &[type_id, index] : type_to_index)
			{
				component_arrays[index]->get_editor_rtti(entity, &editor_types);
			}
			return editor_types;
		}

		void destroy_component_arrays()
		{
			for (auto *component_array : component_arrays)
			{
				component_array->destroy();
			}
		}

	  private:
		template <size_t... S>
		void m_init(Sequence<S...>)
		{
			(init_component_array<DefaultComponent>(&component_arrays[S], &type_to_index, S), ...);
		}
	};

	template <typename... Component>
	struct WorldT
	{
		EntityManager entity_manager{};
		ComponentManager<Component...> component_manager{};

		template <typename... Ts>
		struct IteratorContainer
		{
			explicit IteratorContainer(WorldT *world) : m_world(world) {}
			struct EntityIterator
			{
				EntityIterator(WorldT *world, const Hashmap<Entity, EntityInfo>::HIterator &iterator)
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

				bool matches_signature() { return !is_end() && (m_iterator->value.signature & m_signature) == m_signature; }
				bool is_end() { return m_iterator == m_world->entity_manager.m_living_entities.end(); }

				Hashmap<Entity, EntityInfo>::HIterator m_iterator;
				Signature m_signature{};
				WorldT *m_world = nullptr;
			};

			EntityIterator begin() { return EntityIterator(m_world, m_world->entity_manager.m_living_entities.begin()); }
			EntityIterator end() { return EntityIterator(m_world, m_world->entity_manager.m_living_entities.end()); }

			WorldT *m_world = nullptr;
		};
	};

} // namespace Vultr
