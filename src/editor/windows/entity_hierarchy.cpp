#include "../editor.h"

namespace Vultr
{
	template <typename T>
	static String serialize_bytes(const byte *src, u32 width)
	{
		String res{};
		for (u32 i = 0; i < width; i++)
		{
			T val = *reinterpret_cast<const T *>(src + sizeof(T) * i);
			if constexpr (is_same<T, f32> || is_same<T, f64>)
			{
				res += serialize_f64(val);
			}
			else if (is_same<T, u8> || is_same<T, u16> || is_same<T, u32> || is_same<T, u64>)
			{
				res += serialize_u64(val);
			}
			else if (is_same<T, s8> || is_same<T, s16> || is_same<T, s32> || is_same<T, s64>)
			{
				res += serialize_s64(val);
			}
			if (i != width - 1)
				res += ",";
		}
		return res;
	}

	static String serialize_member(const byte *uniform_data, const Platform::UniformMember &member)
	{
		auto offset     = member.offset;
		const byte *src = uniform_data + offset;
		switch (member.type.primitive_type)
		{
			case PrimitiveType::VEC2:
				return serialize_bytes<f32>(src, 2);
			case PrimitiveType::VEC3:
				return serialize_bytes<f32>(src, 3);
			case PrimitiveType::VEC4:
			case PrimitiveType::COLOR:
				return serialize_bytes<f32>(src, 4);
			case PrimitiveType::MAT3:
				return serialize_bytes<f32>(src, 3 * 3);
			case PrimitiveType::MAT4:
				return serialize_bytes<f32>(src, 4 * 4);
			case PrimitiveType::F32:
				return serialize_bytes<f32>(src, 1);
			case PrimitiveType::F64:
				return serialize_bytes<f64>(src, 1);
			case PrimitiveType::S8:
				return serialize_bytes<s8>(src, 1);
			case PrimitiveType::S16:
				return serialize_bytes<s16>(src, 1);
			case PrimitiveType::S32:
				return serialize_bytes<s32>(src, 1);
			case PrimitiveType::S64:
				return serialize_bytes<s64>(src, 1);
			case PrimitiveType::U8:
				return serialize_bytes<u8>(src, 1);
			case PrimitiveType::U16:
				return serialize_bytes<u16>(src, 1);
			case PrimitiveType::U32:
				return serialize_bytes<u32>(src, 1);
			case PrimitiveType::U64:
				return serialize_bytes<u64>(src, 1);
			default:
				THROW("Invalid uniform member type!");
		}
		return {};
	}

	static ErrorOr<void> serialize_material(Project *project, const Resource<Platform::Material *> &material)
	{
		TRY_UNWRAP(auto *mat, material.try_value());
		TRY_UNWRAP(auto *shader, mat->source.try_value());

		Platform::UUID_String shader_uuid;
		Platform::stringify_uuid(mat->source.id.value(), shader_uuid);

		String out_buf{};
		out_buf += shader_uuid;

		auto *reflection = Platform::get_reflection_data(shader);
		for (auto &uniform_member : reflection->uniform_members)
		{
			out_buf += "\n" + uniform_member.name + ":" + serialize_member(mat->uniform_data, uniform_member);
		}

		u32 i = 0;
		for (auto &sampler_refl : reflection->samplers)
		{
			auto &sampler = mat->samplers[i];
			if (sampler.empty())
			{
				out_buf += "\n" + sampler_refl.name + ":" + Platform::VULTR_NULL_FILE_HANDLE;
			}
			else
			{
				Platform::UUID_String sampler_uuid;
				Platform::stringify_uuid(sampler.id.value(), sampler_uuid);
				out_buf += "\n" + sampler_refl.name + ":" + sampler_uuid;
			}
			i++;
		}

		auto editor_path = project->asset_map.get(material.id.value());
		TRY(try_fwrite_all(editor_path, out_buf, StreamWriteMode::OVERWRITE));

		return Success;
	}

	static void render_entity_hierarchy(Editor *e, Entity entity)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth;

		if (get_children(entity).size() == 0)
			flags |= ImGuiTreeNodeFlags_Leaf;

		bool already_selected = e->selected_entity.has_value() && e->selected_entity.value() == entity;

		if (already_selected)
			flags |= ImGuiTreeNodeFlags_Selected;

		ImGui::PushID(static_cast<int>(entity));

		bool open = ImGui::TreeNodeEx((void *)(u64)(entity), flags, "%s", get_label(entity)->c_str());
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() && !already_selected)
		{
			if (e->selected_entity.has_value() && has_component<Material>(e->selected_entity.value()))
			{
				auto &mat_component = get_component<Material>(e->selected_entity.value());
				if (mat_component.source.loaded())
				{
					auto res = serialize_material(&e->project, mat_component.source);
					if (res.is_error())
						fprintf(stderr, "Something went wrong saving material %s", res.get_error().message.c_str());

					begin_resource_import(e);
				}
			}
			e->selected_entity = entity;
		}

		if (has_component<Transform>(entity))
		{
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("Entity", &entity, sizeof(entity));
				ImGui::Text("Drag drop entity");
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				const auto *payload = ImGui::AcceptDragDropPayload("Entity");
				if (payload != nullptr)
				{
					auto new_child = *static_cast<Entity *>(payload->Data);

					reparent_entity(new_child, entity);
				}
				ImGui::EndDragDropTarget();
			}
		}

		bool deleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (has_parent(entity))
			{
				if (ImGui::Selectable("Unparent"))
				{
					unparent(entity);
					ImGui::CloseCurrentPopup();
				}
			}
			if (ImGui::Selectable("Delete"))
			{
				if (e->selected_entity == entity)
				{
					e->selected_entity = None;
				}
				destroy_entity(entity);
				ImGui::CloseCurrentPopup();
				deleted = true;
			}
			ImGui::EndPopup();
		}

		if (open)
		{
			if (!deleted)
			{
				for (auto child : get_children(entity))
				{
					render_entity_hierarchy(e, child);
				}
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void entity_hierarchy_window_draw(Editor *e)
	{
		ImGui::Begin("Hierarchy");
		if (e->hot_reload_fence.try_acquire())
		{
			for (Entity entity = 1; entity < MAX_ENTITIES; entity++)
			{
				if (!entity_exists(entity))
					continue;

				if (has_parent(entity))
					continue;

				render_entity_hierarchy(e, entity);
			}

			if (ImGui::Button("Create Entity"))
			{
				ImGui::OpenPopup("EntityCreationPopup");
			}

			if (ImGui::BeginPopup("EntityCreationPopup"))
			{
				if (ImGui::Selectable("Mesh"))
				{
					StringView label = "Mesh Entity";
					Entity entity;
					if let (auto parent, e->selected_entity)
					{
						if (has_component<Transform>(parent))
						{
							entity = create_parented_entity(label, parent, Transform{}, Material{}, Mesh{});
						}
						else
						{
							entity = create_entity(label, Transform{}, Material{}, Mesh{});
						}
					}
					else
					{
						entity = create_entity(label, Transform{}, Material{}, Mesh{});
					}
					e->selected_entity = entity;
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::Selectable("Empty"))
				{
					StringView label = "Empty Entity";
					Entity entity;
					if let (auto parent, e->selected_entity)
					{
						if (has_component<Transform>(parent))
						{
							entity = create_parented_entity(label, parent, Transform{});
						}
						else
						{
							entity = create_entity(label, Transform{});
						}
					}
					else
					{
						entity = create_entity(label, Transform{});
					}
					e->selected_entity = entity;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			e->hot_reload_fence.release();
		}
		ImGui::End();
	}
} // namespace Vultr