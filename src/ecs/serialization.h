#pragma once
#include "world.h"
#include <utils/endian.h>
#include <filesystem/filestream.h>
#include <yaml-cpp/yaml.h>
#include <core/resource_allocator/resource_allocator.h>

namespace Vultr
{
	// Stands for Vultr Scene File Format
	static constexpr byte MAGIC_NUMBER[8] = {'V', 'U', 'L', 'T', 'R', 'S', 'F', 'F'};
	static constexpr u16 VERSION          = 1;

	struct SceneHeader
	{
		u64 magic_number              = *(u64 *)MAGIC_NUMBER;
		u16 version                   = VERSION;
		u8 endianness                 = static_cast<u8>(Utils::get_endianness());
		u64 component_registry_offset = sizeof(SceneHeader);
		u64 entity_manager_offset     = 0;
		u64 component_manager_offset  = 0;
	};

	template <typename... Component>
	inline ErrorOr<void> serialize_world_binary(const WorldT<Component...> *world, FileOutputStream *out)
	{
		SceneHeader header{};
		TRY(out->write((byte *)&header, sizeof(header)))
		return Success;
	}

	template <typename... Component>
	inline ErrorOr<void> serialize_world_yaml(WorldT<Component...> *world, FileOutputStream *output_stream)
	{
		YAML::Emitter o{};

		// Output version
		o << YAML::BeginMap;
		o << YAML::Key << "Version";
		o << YAML::Value << VERSION;

		auto *component_manager       = &world->component_manager;
		EntityManager *entity_manager = &world->entity_manager;

		{
			o << YAML::Key << "Component Manager";

			o << YAML::Value;
			o << YAML::BeginMap;

			auto types = component_manager->get_component_rtti();
			for (auto &[type, index] : types)
			{
				IComponentArray *component_array = component_manager->component_arrays[index];

				if (component_array->m_size == 0)
					continue;

				o << YAML::Key << type.id;
				o << YAML::Value;
				o << YAML::BeginMap;

				auto *component = static_cast<byte *>(component_array->m_array);
				for (size_t i = 0; i < component_array->m_size; i++)
				{
					auto entity = component_array->m_index_to_entity.get(i);

					o << YAML::Key << world_get_uuid(world, entity);

					o << YAML::Value << YAML::BeginMap;
					for (auto &field : type.get_fields())
					{
						if (field.type.serialize == nullptr)
						{
							fprintf(stderr, "No serialization function found!\n");
							continue;
						}
						else
						{
							o << YAML::Key << field.id;
							o << YAML::Value;
							field.type.serialize(o, Tuple<Field, void *>(field, component));
						}
					}
					o << YAML::EndMap;

					component += type.size();
				}

				o << YAML::EndMap;
			}

			o << YAML::EndMap;
		}
		{
			o << YAML::Key << "Entity Manager";
			o << YAML::Value << YAML::BeginMap;

			for (auto &[entity, info] : entity_manager->m_living_entities)
			{
				o << YAML::Key << world_get_uuid(world, entity);
				o << YAML::Value << YAML::BeginMap;

				o << YAML::Key << "Label";
				o << YAML::Value << info.label;

				o << YAML::Key << "Parent";
				if (info.parent != INVALID_ENTITY)
					o << YAML::Value << world_get_uuid(world, info.parent);
				else
					o << YAML::Null;

				o << YAML::Key << "Children";

				HashTable<UUID> children{};
				for (auto child : info.children)
				{
					children.set(world_get_uuid(world, child));
				}
				o << YAML::Value << children;

				o << YAML::EndMap;
			}

			o << YAML::EndMap;
		}

		o << YAML::EndMap;

		TRY(output_stream->write(o.c_str(), o.size()));
		TRY(output_stream->write(String("\n")));
		return Success;
	}

	template <typename... Component>
	inline void read_world_binary(const Buffer &src, WorldT<Component...> *out)
	{
	}

	template <typename... Component>
	inline ErrorOr<void> read_world_yaml(const StringView &src, WorldT<Component...> *out)
	{
		YAML::Node root = YAML::Load(src);
		if (!root.IsMap())
			return Error("Invalid Scene Format!");

		auto version = root["Version"].as<u16>();
		auto em_node = root["Entity Manager"];
		auto cm_node = root["Component Manager"];

		if (!cm_node)
			return Error("No component manager in scene source!");

		if (!em_node)
			return Error("No entity manager in scene source!");

		auto *entity_manager    = &out->entity_manager;
		auto *component_manager = &out->component_manager;

		for (const auto &entity_node : em_node)
		{
			UUID uuid         = entity_node.first.as<UUID>();
			Entity new_entity = entity_manager->m_queue.pop();

			EntityInfo info   = {.label = String(entity_node.second["Label"].Scalar().c_str()), .uuid = uuid};
			entity_manager->m_living_entities.set(new_entity, info);
			entity_manager->m_uuid_to_entity.set(uuid, new_entity);
		}

		for (const auto &entity_node : em_node)
		{
			Entity entity    = entity_manager->m_uuid_to_entity.get(entity_node.first.as<UUID>());

			EntityInfo *info = &entity_manager->m_living_entities.get(entity);
			{
				for (UUID child_uuid : entity_node.second["Children"].as<Vector<UUID>>())
				{
					if (!entity_manager->m_uuid_to_entity.contains(child_uuid))
						return Error("Invalid child id!");

					info->children.set<Entity>(entity_manager->m_uuid_to_entity.get(child_uuid));
				}
			}
			if (entity_node.second["Parent"].IsNull())
				continue;

			UUID parent_uuid = entity_node.second["Parent"].as<UUID>();

			if (!entity_manager->m_uuid_to_entity.contains(parent_uuid))
				return Error("Invalid parent id!");

			info->parent = entity_manager->m_uuid_to_entity.get(parent_uuid);
		}

		{
			HashTable<u32> used_components{};
			for (const auto &component_array : cm_node)
			{
				u32 type_id = component_array.first.as<u32>();

				if (used_components.contains(type_id))
					return Error("Component exists twice!");

				used_components.set<u32>(type_id);

				if (!component_manager->type_to_index.contains(type_id))
					continue;

				u32 index              = component_manager->type_to_index.get(type_id);
				IComponentArray *array = component_manager->component_arrays[index];

				Signature signature{};
				signature.set(index, true);

				Type type = array->rtti_type;
				Hashmap<u32, Field *> id_to_field{};
				for (auto &field : type.get_fields())
				{
					id_to_field.set(field.id, &field);
				}

				for (const auto &component : component_array.second)
				{
					Entity entity = entity_manager->m_uuid_to_entity.get(component.first.as<UUID>());

					entity_manager->add_signature(entity, signature);
					void *component_memory;

					{
						auto new_index = array->m_size;
						array->m_entity_to_index.set(entity, new_index);
						array->m_index_to_entity.set(new_index, entity);
						component_memory = static_cast<byte *>(array->m_array) + (new_index * type.size());
						array->m_size++;
					}

					HashTable<u32> unused_fields{};
					for (auto &field : type.get_fields())
						unused_fields.set<u32>(field.id);

					for (const auto &field_node : component.second)
					{
						u32 field_id   = field_node.first.as<u32>();
						auto data_node = field_node.second;

						if (id_to_field.contains(field_id))
						{
							auto *field = id_to_field.get(field_id);
							field->initialize_default(component_memory);
							field->deserialize(data_node, component_memory);
							unused_fields.remove(field_id);
						}
					}

					for (u32 unused_field_id : unused_fields)
					{
						auto *field = id_to_field.get(unused_field_id);
						field->initialize_default(component_memory);
					}
				}
			}
		}

		return Success;
	}
} // namespace Vultr