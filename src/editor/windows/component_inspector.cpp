#include "../editor.h"
#include <glm/gtc/type_ptr.hpp>

namespace Vultr
{
	template <typename T>
	static void draw_resource_target(const EditorField &editor_field)
	{
		ImGui::PushID(editor_field.field.name);
		auto *resource = editor_field.get_addr<Resource<T>>();
		ImGui::Selectable("##", false, 0, ImVec2(20, 20));
		if (ImGui::BeginDragDropTarget())
		{
			const char *resource_type_string;
			switch (editor_field.field.type.primitive_type)
			{
				case PrimitiveType::TEXTURE_RESOURCE:
					resource_type_string = resource_type_to_string(ResourceType::TEXTURE);
					break;
				case PrimitiveType::MESH_RESOURCE:
					resource_type_string = resource_type_to_string(ResourceType::MESH);
					break;
				case PrimitiveType::SHADER_RESOURCE:
					resource_type_string = resource_type_to_string(ResourceType::SHADER);
					break;
				case PrimitiveType::MATERIAL_RESOURCE:
					resource_type_string = resource_type_to_string(ResourceType::MATERIAL);
					break;
				default:
					resource_type_string = resource_type_to_string(ResourceType::OTHER);
					break;
			}

			const auto *payload = ImGui::AcceptDragDropPayload(resource_type_string);

			if (payload != nullptr)
			{
				UUID asset_uuid;
				memcpy(asset_uuid.m_uuid, payload->Data, payload->DataSize);

				auto new_res = Resource<T>(asset_uuid);
				*resource    = new_res;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::SameLine();
		if (!resource->empty())
		{
			Platform::UUID_String uuid_string;
			Platform::stringify_uuid(resource->id.value(), uuid_string);
			ImGui::Text("%s: %s", editor_field.field.name.c_str(), uuid_string);
		}
		else
		{
			ImGui::Text("%s", editor_field.field.name.c_str());
		}

		ImGui::SameLine();

		if (ImGui::Button("Remove resource"))
		{
			*resource = Resource<T>();
		}

		if (resource->has_error())
		{
			ImGui::Text("Failed to load: %s", resource->get_error().message.c_str());
		}
		ImGui::PopID();
	}
	static void draw_editor_field(const EditorField &editor_field)
	{
		auto &field = editor_field.field;
		auto *addr  = editor_field.addr;
		switch (field.type.primitive_type)
		{
			case PrimitiveType::U8:
				ImGui::DragScalar(field.name, ImGuiDataType_U8, addr, 1);
				break;
			case PrimitiveType::U16:
				ImGui::DragScalar(field.name, ImGuiDataType_U16, addr, 1);
				break;
			case PrimitiveType::U32:
				ImGui::DragScalar(field.name, ImGuiDataType_U32, addr, 1);
				break;
			case PrimitiveType::U64:
				ImGui::DragScalar(field.name, ImGuiDataType_U64, addr, 1);
				break;
			case PrimitiveType::S8:
				ImGui::DragScalar(field.name, ImGuiDataType_S8, addr, 1);
				break;
			case PrimitiveType::S16:
				ImGui::DragScalar(field.name, ImGuiDataType_S16, addr, 1);
				break;
			case PrimitiveType::S32:
				ImGui::DragScalar(field.name, ImGuiDataType_S32, addr, 1);
				break;
			case PrimitiveType::S64:
				ImGui::DragScalar(field.name, ImGuiDataType_S64, addr, 1);
				break;
			case PrimitiveType::F32:
				ImGui::DragScalar(field.name, ImGuiDataType_Float, addr, 0.02f);
				break;
			case PrimitiveType::F64:
				ImGui::DragScalar(field.name, ImGuiDataType_Double, addr, 0.02f);
				break;
			case PrimitiveType::VECTOR:
			case PrimitiveType::BITFIELD:
			case PrimitiveType::ARRAY:
			case PrimitiveType::HASHMAP:
			case PrimitiveType::HASHTABLE:
			case PrimitiveType::STRING_HASH:
			case PrimitiveType::BUFFER:
			case PrimitiveType::QUEUE:
			case PrimitiveType::MAT3:
			case PrimitiveType::MAT4:
				break;
			case PrimitiveType::CHAR:
				ImGui::Text("%s Char %c", field.name.c_str(), *static_cast<char *>(addr));
				break;
			case PrimitiveType::BYTE:
				ImGui::Text("%s Byte %u", field.name.c_str(), *static_cast<byte *>(addr));
				break;
			case PrimitiveType::BOOL:
				ImGui::Checkbox(field.name, static_cast<bool *>(addr));
				break;
			case PrimitiveType::UUID:
			{
				Platform::UUID_String buf;
				Platform::stringify_uuid(*static_cast<UUID *>(addr), buf);
				ImGui::Text("%s UUID %s", field.name.c_str(), buf);
				break;
			}
			case PrimitiveType::STRING_VIEW:
				ImGui::Text("%s String %s", field.name.c_str(), static_cast<StringView *>(addr)->c_str());
				break;
			case PrimitiveType::PTR:
				ImGui::Text("%s void * %p", field.name.c_str(), addr);
				break;
			case PrimitiveType::VEC2:
			{
				Vec2 *vec2 = static_cast<Vec2 *>(addr);
				ImGui::Text("%s", field.name.c_str());
				ImGui::SameLine();

				ImGui::PushID((field.name + ".x").c_str());
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &vec2->x, 0.02f);
				ImGui::SameLine();
				ImGui::PopID();

				ImGui::PushID((field.name + ".y").c_str());
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &vec2->y, 0.02f);
				ImGui::SameLine();
				ImGui::PopID();
				break;
			}
			case PrimitiveType::VEC3:
			{
				Vec3 *vec3 = static_cast<Vec3 *>(addr);
				ImGui::PushID((field.name + ".x").c_str());
				ImGui::Text("%s", field.name.c_str());
				ImGui::SameLine();
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &vec3->x, 0.02f);
				ImGui::SameLine();
				ImGui::PopID();

				ImGui::PushID((field.name + ".y").c_str());
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &vec3->y, 0.02f);
				ImGui::SameLine();
				ImGui::PopID();

				ImGui::PushID((field.name + ".z").c_str());
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &vec3->z, 0.02f);
				ImGui::PopID();
				break;
			}
			case PrimitiveType::VEC4:
			{
				Vec4 *vec4 = static_cast<Vec4 *>(addr);
				ImGui::PushID((field.name + ".x").c_str());
				ImGui::Text("%s", field.name.c_str());
				ImGui::SameLine();
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &vec4->x, 0.02f);
				ImGui::SameLine();
				ImGui::PopID();

				ImGui::PushID((field.name + ".y").c_str());
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &vec4->y, 0.02f);
				ImGui::SameLine();
				ImGui::PopID();

				ImGui::PushID((field.name + ".z").c_str());
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &vec4->z, 0.02f);
				ImGui::SameLine();
				ImGui::PopID();

				ImGui::PushID((field.name + ".w").c_str());
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &vec4->w, 0.02f);
				ImGui::PopID();
				break;
			}
			case PrimitiveType::COLOR:
			{
				f32 *val = glm::value_ptr(*static_cast<Vec4 *>(addr));
				ImGui::ColorEdit4(field.name, val);
				break;
			}
			case PrimitiveType::QUAT:
			{
				Quat *quat = static_cast<Quat *>(addr);

				ImGui::PushID((field.name + ".x").c_str());
				ImGui::Text("%s", field.name.c_str());
				ImGui::SameLine();
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &quat->x, 0.02f);
				ImGui::SameLine();
				ImGui::PopID();

				ImGui::PushID((field.name + ".y").c_str());
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &quat->y, 0.02f);
				ImGui::SameLine();
				ImGui::PopID();

				ImGui::PushID((field.name + ".z").c_str());
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &quat->z, 0.02f);
				ImGui::SameLine();
				ImGui::PopID();

				ImGui::PushID((field.name + ".w").c_str());
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("", &quat->w, 0.02f);
				ImGui::PopID();

				break;
			}
			case PrimitiveType::PATH:
				break;
			case PrimitiveType::STRING:
				break;
			case PrimitiveType::TEXTURE_RESOURCE:
				draw_resource_target<Platform::Texture *>(editor_field);
				break;
			case PrimitiveType::MESH_RESOURCE:
				draw_resource_target<Platform::Mesh *>(editor_field);
				break;
			case PrimitiveType::SHADER_RESOURCE:
				draw_resource_target<Platform::Shader *>(editor_field);
				break;
			case PrimitiveType::MATERIAL_RESOURCE:
				draw_resource_target<Platform::Material *>(editor_field);
				break;
			case PrimitiveType::NONE:
				break;
		}
	}

	static void draw_component(Editor *e, const Type &type, const Vector<EditorField> &fields, void *data)
	{
		auto *state = &e->component_inspector;
		auto *cm    = &world()->component_manager;
		if (ImGui::CollapsingHeader(type.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(type.name.c_str());
			if (ImGui::Button("Reset"))
			{
				for (auto &field : type.get_fields())
				{
					field.initialize_default(data);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Copy"))
			{
				state->component_clipboard.resize(type.size());
				state->component_clipboard.fill(static_cast<byte *>(data), type.size());
			}
			if (state->component_clipboard.size() == type.size())
			{
				ImGui::SameLine();
				if (ImGui::Button("Paste"))
				{
					type.copy_constructor(data, state->component_clipboard.storage);
				}
			}

			for (auto editor_field : fields)
			{
				draw_editor_field(editor_field);
			}

			if (type.id != get_type<Transform>.id && ImGui::Button("Remove"))
			{
				auto index = cm->type_to_index.get(type.id);
				cm->component_arrays[index]->remove_entity(e->selected_entity.value());
				Signature component_signature;
				component_signature.set(index, true);
				world()->entity_manager.remove_signature(e->selected_entity.value(), component_signature);
			}

			ImGui::PopID();
		}
	}

	void component_inspector_window_draw(Editor *e)
	{
		ImGui::Begin("Inspector");
		if (e->hot_reload_fence.try_acquire())
		{
			if (e->selected_entity)
			{
				ImGui::InputText("Label", get_label(e->selected_entity.value()));

				auto *cm  = &world()->component_manager;

				auto info = cm->get_component_editor_rtti(e->selected_entity.value());

				for (auto [type, fields, data] : info)
				{
					draw_component(e, type, fields, data);
				}

				if (ImGui::Button("Add Component"))
				{
					ImGui::OpenPopup("ComponentAddPopup");
				}

				if (ImGui::BeginPopup("ComponentAddPopup"))
				{
					for (auto [type_id, index] : cm->type_to_index)
					{
						auto *array = cm->component_arrays[index];
						auto &type  = array->rtti_type;

						Signature component_signature;
						component_signature.set(index, true);
						const auto &signature = get_signature(e->selected_entity.value());

						if (signature.at(index))
							continue;

						if (ImGui::Selectable(type.name.c_str()))
						{

							auto new_index = array->m_size;
							array->m_entity_to_index.set(e->selected_entity.value(), new_index);
							array->m_index_to_entity.set(new_index, e->selected_entity.value());
							auto *component_memory = static_cast<byte *>(array->m_array) + (new_index * type.size());
							array->m_size++;

							for (auto &field : type.get_fields())
							{
								field.initialize_default(component_memory);
							}

							world()->entity_manager.add_signature(e->selected_entity.value(), component_signature);
							ImGui::CloseCurrentPopup();
						}
					}
					ImGui::EndPopup();
				}
			}
			e->hot_reload_fence.release();
		}
		ImGui::End();
	}
} // namespace Vultr