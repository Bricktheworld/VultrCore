#include "windows.h"
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo/ImGuizmo.h>
#include <math/decompose_transform.h>
#include <editor/res/resources.h>
#include <ecs/serialization.h>

namespace ImGui
{
	bool InputText(const Vultr::StringView &label, Vultr::String *string)
	{
		size_t buf_size = string->size() + 256;
		char buf[buf_size];
		strncpy(buf, string->c_str(), string->size());
		if (ImGui::InputText(label.c_str(), buf, buf_size))
		{
			*string = buf;
			return true;
		}
		return false;
	}

	void ErrorModal(const Vultr::StringView &label, const Vultr::StringView &message)
	{
		if (ImGui::BeginPopupModal(label.c_str()))
		{
			ImGui::Text("An error has occurred!");
			ImGui::Text("%s", message.c_str());
			if (ImGui::Button("OK"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}
} // namespace ImGui
namespace Vultr
{
	void scene_window_update(EditorWindowState *state, f64 dt)
	{
		if (Platform::mouse_down(engine()->window, Platform::Input::MouseButton::MOUSE_RIGHT))
		{
			Platform::lock_cursor(engine()->window);

			static constexpr f32 speed = 2;
			static constexpr f32 sens  = 100000;
			f32 delta                  = speed * (f32)dt;
			auto &transform            = state->editor_camera_transform;

			if (Platform::key_down(engine()->window, Platform::Input::KEY_W))
				transform.position += forward(transform) * delta;
			if (Platform::key_down(engine()->window, Platform::Input::KEY_S))
				transform.position -= forward(transform) * delta;
			if (Platform::key_down(engine()->window, Platform::Input::KEY_D))
				transform.position += right(transform) * delta;
			if (Platform::key_down(engine()->window, Platform::Input::KEY_A))
				transform.position -= right(transform) * delta;
			if (Platform::key_down(engine()->window, Platform::Input::KEY_E))
				transform.position += Vec3(0, 1, 0) * delta;
			if (Platform::key_down(engine()->window, Platform::Input::KEY_Q))
				transform.position -= Vec3(0, 1, 0) * delta;

			auto mouse_delta    = Platform::get_mouse_delta(engine()->window);

			f64 aspect_ratio    = (f64)Platform::get_window_width(engine()->window) / (f64)Platform::get_window_width(engine()->window);
			Quat rotation_horiz = glm::angleAxis(f32(sens * dt * -mouse_delta.x * aspect_ratio), Vec3(0, 1, 0));
			Quat rotation_vert  = glm::angleAxis(f32(sens * dt * -mouse_delta.y), right(transform));
			transform.rotation  = rotation_horiz * rotation_vert * transform.rotation;
		}
		else
		{
			Platform::unlock_cursor(engine()->window);
		}
	}

	static ErrorOr<Path> local_resource_file(const Project *project, const Path &full_path)
	{
		if (starts_with(full_path.string(), project->resource_dir.string()))
		{
			return Path(full_path.string().substr(project->resource_dir.string().length()));
		}
		else if (starts_with(full_path.string(), project->build_resource_dir.string()))
		{
			return Path(full_path.string().substr(project->build_resource_dir.string().length()));
		}
		else
		{
			return Error("Not a valid resource!");
		}
	}

	static bool resource_is_imported(Project *project, const Path &full_path)
	{
		if check (local_resource_file(project, full_path), auto path, auto err)
		{
			switch (get_resource_type(path))
			{
				case ResourceType::TEXTURE:
				{
					return exists(get_texture_resource(project, path));
				}
				case ResourceType::SHADER:
				{
					auto [vert, frag] = get_shader_resource(project, path);
					return exists(vert) && exists(frag);
				}
				case ResourceType::MATERIAL:
				{
					return exists(get_material_resource(project, path));
				}
				case ResourceType::MESH:
				{
					auto [vert, index] = get_mesh_resource(project, path);
					return exists(vert) && exists(index);
				}
				case ResourceType::SCENE:
					return exists(get_scene_resource(project, path));
				case ResourceType::CPP_SRC:
				case ResourceType::OTHER:
					return false;
			}
		}
		else
		{
			return false;
		}
	}

	void scene_window_draw(EditorWindowState *state, EditorRuntime *runtime)
	{
		ImGui::Begin("Game");
		ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
		auto output_texture        = Platform::imgui_get_texture_id(Platform::get_attachment_texture(runtime->render_system->output_framebuffer, 0));
		ImGui::Image(output_texture, viewport_panel_size);
		if (ImGui::BeginDragDropTarget())
		{
			const auto *payload = ImGui::AcceptDragDropPayload("File");

			if (payload != nullptr)
			{
				auto file = Path(static_cast<char *>(payload->Data));
				if (get_resource_type(file) == ResourceType::SCENE)
				{
					state->scene_path = file;
					String src;
					fread_all(file, &src);
					printf("Freeing scene......................\n");
					world()->component_manager.destroy_component_arrays();
					world()->entity_manager = EntityManager();
					printf("Initializing new scene.............\n");
					CHECK(read_world_yaml(src, world()));
				}
				else
				{
					ImGui::OpenPopup("Invalid Resource");
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::ErrorModal("Invalid Resource", "Resource provided is invalid!");

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();

		f32 window_width  = (f32)ImGui::GetWindowWidth();
		f32 window_height = (f32)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, window_width, window_height);

		if (state->selected_entity.has_value())
		{
			auto ent = state->selected_entity.value();
			if (has_component<Transform>(ent))
			{
				auto transform_mat = get_world_transform(ent);
				auto view_mat      = view_matrix(state->editor_camera_transform);
				auto camera_proj   = projection_matrix(state->editor_camera, window_width, window_height);

				bool snap          = Platform::key_down(engine()->window, Platform::Input::KEY_LEFT_SHIFT);
				f32 snap_value     = state->current_operation != ImGuizmo::OPERATION::ROTATE ? 0.5f : 45.0f;
				f32 snap_values[]  = {snap_value, snap_value, snap_value};

				ImGuizmo::Manipulate(glm::value_ptr(view_mat), glm::value_ptr(camera_proj), (ImGuizmo::OPERATION)state->current_operation, ImGuizmo::LOCAL, glm::value_ptr(transform_mat), nullptr,
									 snap ? snap_values : nullptr);

				if (ImGuizmo::IsUsing())
				{
					auto &transform = get_component<Transform>(ent);
					Math::decompose_transform(get_local_transform(transform_mat, ent), &transform.position, &transform.rotation, &transform.scale);
				}
			}
		}

		ImGui::End();
	}

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

	static ErrorOr<void> serialize_material(const Path &editor_res_path, const Resource<Platform::Material *> &material)
	{
		auto *mat_allocator     = resource_allocator<Platform::Material *>();
		auto *shader_allocator  = resource_allocator<Platform::Shader *>();
		auto *texture_allocator = resource_allocator<Platform::Texture *>();
		TRY_UNWRAP(auto *mat, material.try_value());
		TRY_UNWRAP(auto *shader, mat->source.try_value());
		auto shader_path = shader_allocator->get_resource_path(ResourceId(mat->source).id);

		String out_buf{};
		out_buf += shader_path.string();

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
				auto sampler_path = texture_allocator->get_resource_path(ResourceId(sampler).id);

				out_buf += "\n" + sampler_refl.name + ":" + sampler_path.string();
			}
			i++;
		}

		auto mat_path = mat_allocator->get_resource_path(ResourceId(material).id);
		TRY(try_fwrite_all(editor_res_path / mat_path, out_buf, StreamWriteMode::OVERWRITE));

		return Success;
	}

	static void render_entity_hierarchy(Entity entity, EditorWindowState *state, Project *project)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth;

		if (get_children(entity).size() == 0)
			flags |= ImGuiTreeNodeFlags_Leaf;

		bool already_selected = state->selected_entity.has_value() && state->selected_entity.value() == entity;

		if (already_selected)
			flags |= ImGuiTreeNodeFlags_Selected;

		ImGui::PushID(static_cast<int>(entity));

		bool open = ImGui::TreeNodeEx((void *)(u64)(entity), flags, "%s", get_label(entity)->c_str());
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() && !already_selected)
		{
			if (state->selected_entity.has_value() && has_component<Material>(state->selected_entity.value()))
			{
				auto &mat_component = get_component<Material>(state->selected_entity.value());
				if (mat_component.source.loaded())
				{
					auto res = serialize_material(project->resource_dir, mat_component.source);
					if (res.is_error())
						fprintf(stderr, "Something went wrong saving material %s", res.get_error().message.c_str());

					begin_resource_import(project, state);
				}
			}
			state->selected_entity = entity;
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
				if (state->selected_entity == entity)
				{
					state->selected_entity = None;
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
					render_entity_hierarchy(child, state, project);
				}
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void entity_hierarchy_window_draw(Project *project, EditorWindowState *state)
	{
		ImGui::Begin("Hierarchy");
		for (Entity entity = 1; entity < MAX_ENTITIES; entity++)
		{
			if (!entity_exists(entity))
				continue;

			if (has_parent(entity))
				continue;

			render_entity_hierarchy(entity, state, project);
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
				if let (auto parent, state->selected_entity)
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
				state->selected_entity = entity;
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Selectable("Empty"))
			{
				StringView label = "Empty Entity";
				Entity entity;
				if let (auto parent, state->selected_entity)
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
				state->selected_entity = entity;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::End();
	}

	template <typename T>
	static void draw_resource_target(Project *project, const EditorField &editor_field)
	{
		ImGui::PushID(editor_field.field.name);
		auto *resource = editor_field.get_addr<Resource<T>>();
		ImGui::Selectable("##", false, 0, ImVec2(20, 20));
		if (ImGui::BeginDragDropTarget())
		{
			const auto *payload = ImGui::AcceptDragDropPayload("File");

			if (payload != nullptr)
			{
				Path full_path = Path(String(static_cast<char *>(payload->Data), payload->DataSize));
				if check (local_resource_file(project, full_path), auto file, auto err)
				{
					auto payload_type = get_resource_type(file);

					bool matches      = false;
					switch (editor_field.field.type.primitive_type)
					{
						case PrimitiveType::TEXTURE_RESOURCE:
							matches = payload_type == ResourceType::TEXTURE;
							break;
						case PrimitiveType::MESH_RESOURCE:
							matches = payload_type == ResourceType::MESH;
							break;
						case PrimitiveType::SHADER_RESOURCE:
							matches = payload_type == ResourceType::SHADER;
							break;
						case PrimitiveType::MATERIAL_RESOURCE:
							matches = payload_type == ResourceType::MATERIAL;
							break;
						default:
							break;
					}

					if (resource_is_imported(project, full_path))
					{
						if (matches)
						{
							auto new_res = Resource<T>(file);
							*resource    = new_res;
						}
						else
						{
							ImGui::OpenPopup("Invalid Resource");
						}
					}
					else
					{
						ImGui::OpenPopup("Resource Not Imported");
					}
				}
				else
				{
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::ErrorModal("Invalid Resource", "Resource provided is invalid!");
		ImGui::ErrorModal("Resource Not Imported", "Resource provided has not been imported, please reimport assets!");

		ImGui::SameLine();
		if (!resource->empty())
		{
			auto path = resource_allocator<T>()->get_resource_path(ResourceId(*resource).id);
			ImGui::Text("%s: %s", editor_field.field.name.c_str(), path.c_str());
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
		ImGui::PopID();
	}

	void component_inspector_window_draw(Project *project, EditorWindowState *state)
	{
		ImGui::Begin("Inspector");
		if (state->selected_entity)
		{
			ImGui::InputText("Label", get_label(state->selected_entity.value()));
			auto *cm  = &world()->component_manager;
			auto info = cm->get_component_editor_rtti(state->selected_entity.value());
			for (auto [type, fields, data] : info)
			{
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
								draw_resource_target<Platform::Texture *>(project, editor_field);
								break;
							case PrimitiveType::MESH_RESOURCE:
								draw_resource_target<Platform::Mesh *>(project, editor_field);
								break;
							case PrimitiveType::SHADER_RESOURCE:
								draw_resource_target<Platform::Shader *>(project, editor_field);
								break;
							case PrimitiveType::MATERIAL_RESOURCE:
								draw_resource_target<Platform::Material *>(project, editor_field);
								break;
							case PrimitiveType::NONE:
								break;
						}
					}
					if (type.id != get_type<Transform>.id && ImGui::Button("Remove"))
					{
						auto index = cm->type_to_index.get(type.id);
						cm->component_arrays[index]->remove_entity(state->selected_entity.value());
						Signature component_signature;
						component_signature.set(index, true);
						world()->entity_manager.remove_signature(state->selected_entity.value(), component_signature);
					}

					ImGui::PopID();
				}
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

					if (ImGui::Selectable(type.name.c_str()))
					{
						Signature component_signature;
						component_signature.set(index, true);
						const auto &signature = get_signature(state->selected_entity.value());

						if (signature.at(index))
							continue;

						auto new_index = array->m_size;
						array->m_entity_to_index.set(state->selected_entity.value(), new_index);
						array->m_index_to_entity.set(new_index, state->selected_entity.value());
						auto *component_memory = static_cast<byte *>(array->m_array) + (new_index * type.size());
						array->m_size++;

						for (auto &field : type.get_fields())
						{
							field.initialize_default(component_memory);
						}

						world()->entity_manager.add_signature(state->selected_entity.value(), component_signature);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}
#define WIDGET_SIZE 130.0f
#define ASSET_BROWSER_PADDING 2
#define ICON_SIZE 100.0f

	static u32 get_num_cols()
	{

		s32 num_cols = static_cast<s32>(ImGui::GetWindowWidth()) / WIDGET_SIZE - ASSET_BROWSER_PADDING;

		if (num_cols < 1)
			return 1;

		return static_cast<u32>(num_cols);
	}

	static void draw_directory(const Path &path) {}

	static void change_dir(Project *project, Path dir, EditorWindowState *state)
	{
		state->resource_browser_state.selected_index = None;
		state->resource_browser_state.current_dir    = dir;
		state->resource_browser_state.dirs.clear();
		state->resource_browser_state.files.clear();
		state->resource_browser_state.need_refresh = false;
		for (auto path : DirectoryIterator(dir))
		{
			if (path.is_directory())
			{
				state->resource_browser_state.dirs.push_back(path);
			}
			else
			{
				if (resource_is_imported(project, path))
					state->resource_browser_state.files.push_back(path);
			}
		}
	}

	static void resource_window_init(Project *project, EditorWindowState *state) { change_dir(project, project->resource_dir, state); }

	static void path_drag_drop_src(const Path &path, str type)
	{
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(type, path.c_str(), path.string().length());
			ImGui::EndDragDropSource();
		}
	}

	static Platform::Texture *get_texture_for_resource(const Path &path, EditorWindowState *state)
	{
		switch (get_resource_type(path))
		{
			case ResourceType::TEXTURE:
				return state->texture;
			case ResourceType::SHADER:
				return state->shader;
			case ResourceType::MATERIAL:
				// TODO(Brandon): Get an actual icon for materials.
				return state->file;
			case ResourceType::MESH:
				return state->mesh;
			case ResourceType::CPP_SRC:
				return state->cpp_source;
			case ResourceType::SCENE:
				// TODO(Brandon): Get an actual icon for scenes.
				return state->file;
			case ResourceType::OTHER:
				return state->file;
		}
	}

	static void draw_resource(const Path &path, Platform::Texture *texture)
	{
		auto name           = path.basename();

		auto folder_texture = Platform::imgui_get_texture_id(texture);
		ImGui::SameLine((WIDGET_SIZE - ICON_SIZE) / 2);
		ImGui::Image(folder_texture, ImVec2(ICON_SIZE, ICON_SIZE));

		f32 padding_left = (WIDGET_SIZE - ImGui::CalcTextSize(name.c_str()).x) / 2;

		ImGui::SameLine(padding_left);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ICON_SIZE);

		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + WIDGET_SIZE - padding_left);
		ImGui::Text("%s", name.c_str());
		ImGui::PopTextWrapPos();
	}

	static void resource_browser_refresh(Project *project, EditorWindowState *state)
	{
		if (!state->resource_browser_state.need_refresh)
			return;
		change_dir(project, state->resource_browser_state.current_dir, state);
	}

	void resource_browser_window_draw(Project *project, EditorWindowState *editor_state)
	{
		resource_browser_refresh(project, editor_state);
		ImGui::Begin("Asset Browser");
		u32 num_cols = get_num_cols();
		auto *state  = &editor_state->resource_browser_state;

		if (ImGui::Button("Back"))
		{
			if check (get_parent(state->current_dir), auto parent, auto err)
			{
				change_dir(project, parent, editor_state);
				ImGui::End();
				return;
			}
			else
			{
			}
		}
		ImGui::SameLine();

		String reimport_error_message;
		{
			if (ImGui::Button("Reimport"))
			{
				begin_resource_import(project, editor_state);
				ImGui::End();
				return;
			}
		}

		if (ImGui::BeginTable("Asset Table", static_cast<s32>(num_cols)))
		{
			ImGui::TableNextRow();
			u32 col = 0;
			for (u32 i = 0; i < state->files.size(); i++)
			{
				if (col >= num_cols)
				{
					ImGui::TableNextRow();
					col = 0;
				}
				ImGui::TableSetColumnIndex(static_cast<s32>(col));
				auto &file = state->files[i];

				ImGui::PushID(i);
				if (ImGui::Selectable("##", state->selected_index.has_value() && state->selected_index.value() == i, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(WIDGET_SIZE, WIDGET_SIZE)))
				{
					state->selected_index = i;
				}

				if (ImGui::BeginPopupContextItem())
				{
					if (get_resource_type(file) == ResourceType::SHADER)
					{
						if (ImGui::Selectable("New Material"))
						{
							if (resource_is_imported(project, file))
							{
								CHECK_UNWRAP(auto local_shader_path, local_resource_file(project, file));
								Resource<Platform::Shader *> shader(local_shader_path);
								shader.wait_loaded();

								u32 count    = 1;
								auto new_mat = state->current_dir / "new_material.mat";
								while (exists(new_mat))
								{
									new_mat = state->current_dir / (String("new_material_") + serialize_u64(count) + String(".mat"));
									count++;
								}

								String out_buf{};
								out_buf += local_shader_path.string();

								auto *reflection = Platform::get_reflection_data(shader.value());

								byte buf[Platform::MAX_MATERIAL_SIZE];
								for (auto &uniform_member : reflection->uniform_members)
								{
									out_buf += "\n" + uniform_member.name + ":" + serialize_member(buf, uniform_member);
								}

								for (auto &sampler_refl : reflection->samplers)
								{
									out_buf += "\n" + sampler_refl.name + ":" + Platform::VULTR_NULL_FILE_HANDLE;
								}

								fwrite_all(new_mat, out_buf, StreamWriteMode::OVERWRITE);

								begin_resource_import(project, editor_state);

								//								change_dir(project, state->current_dir, editor_state);

								ImGui::CloseCurrentPopup();
							}
						}
					}
					//					if (ImGui::Selectable("Rename"))
					//					{
					//						ImGui::CloseCurrentPopup();
					//						ImGui::OpenPopup("RenameFilePopup");
					//					}
					ImGui::EndPopup();
				}

				//				if (ImGui::BeginPopup("RenameFilePopup"))
				//				{
				//					String name = String(file.basename());
				//					ImGui::InputText("", &name);
				//					ImGui::EndPopup();
				//				}

				path_drag_drop_src(file, "File");

				draw_resource(file, get_texture_for_resource(file, editor_state));

				ImGui::PopID();

				col++;
			}

			for (u32 i = 0; i < state->dirs.size(); i++)
			{
				if (col >= num_cols)
				{
					ImGui::TableNextRow();
					col = 0;
				}
				ImGui::TableSetColumnIndex(static_cast<s32>(col));
				u32 full_index = i + state->files.size();
				auto &dir      = state->dirs[i];

				ImGui::PushID(full_index);

				if (ImGui::Selectable("##", state->selected_index.has_value() && state->selected_index.value() == full_index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(WIDGET_SIZE, WIDGET_SIZE)))
				{
					state->selected_index = full_index;
				}

				path_drag_drop_src(dir, "Directory");

				if (ImGui::BeginDragDropTarget())
				{
					const auto *payload = ImGui::AcceptDragDropPayload("File");

					if (payload != nullptr)
					{
						auto *path = static_cast<char *>(payload->Data);

						change_dir(project, state->current_dir, editor_state);

						ImGui::PopID();
						ImGui::EndTable();
						ImGui::End();
						return;
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					change_dir(project, dir, editor_state);
					ImGui::PopID();
					ImGui::EndTable();
					ImGui::End();
					return;
				}

				draw_resource(dir, editor_state->folder);

				ImGui::PopID();
				col++;
			}

			ImGui::EndTable();
		}

		if (!reimport_error_message.is_empty())
			ImGui::OpenPopup("Reimport-Error");

		ImGui::ErrorModal("Reimport-Error", reimport_error_message);

		ImGui::End();
	}

	static void serialize_current_scene(EditorWindowState *state)
	{
		if let (const auto &scene_path, state->scene_path)
		{
			auto out = FileOutputStream(state->scene_path.value(), StreamFormat::BINARY, StreamWriteMode::OVERWRITE);
			CHECK(serialize_world_yaml(world(), &out));
		}
		else
		{
		}
	}

	static void on_key_press(void *data, Platform::Input::Key key, Platform::Input::Action action, u32 modifiers)
	{
		auto *state = static_cast<EditorWindowState *>(data);
		if (Platform::mouse_down(engine()->window, Platform::Input::MOUSE_RIGHT))
		{
			if (modifiers == 0)
			{
				if (key == Platform::Input::KEY_Q)
				{
					state->current_operation = ImGuizmo::OPERATION::TRANSLATE;
				}
				else if (Platform::key_down(engine()->window, Platform::Input::KEY_W))
				{
					state->current_operation = ImGuizmo::OPERATION::ROTATE;
				}
				else if (Platform::key_down(engine()->window, Platform::Input::KEY_E))
				{
					state->current_operation = ImGuizmo::OPERATION::SCALE;
				}
			}
		}
		else
		{
			if (action == Platform::Input::PRESS)
			{
				if ((modifiers & Platform::Input::CTRL_BIT) == Platform::Input::CTRL_BIT)
				{
					if (key == Platform::Input::KEY_S)
					{
						serialize_current_scene(state);
					}
					else if (key == Platform::Input::KEY_D)
					{
						if let (auto entity, state->selected_entity)
						{
							Entity duplicate;
							if (has_parent(entity))
							{
								duplicate = create_parented_entity(*get_label(entity), get_parent(entity).value(), Transform{});
							}
							else
							{
								duplicate = create_entity(*get_label(entity));
							}
							auto signature = get_signature(entity);
							auto *em       = &world()->entity_manager;
							auto *cm       = &world()->component_manager;
							em->add_signature(duplicate, signature);

							for (auto [type_id, index] : cm->type_to_index)
							{
								if (!signature.at(index))
									continue;

								auto *array = cm->component_arrays[index];
								auto type   = array->rtti_type;
								void *dest;
								if (!array->m_entity_to_index.contains(duplicate))
								{
									auto component_index = array->m_size;
									array->m_entity_to_index.set(duplicate, component_index);
									array->m_index_to_entity.set(component_index, duplicate);
									dest = static_cast<byte *>(array->m_array) + (component_index * type.size());
									array->m_size++;
								}
								else
								{
									auto component_index = array->m_entity_to_index.get(duplicate);
									dest                 = static_cast<byte *>(array->m_array) + (component_index * type.size());
								}
								void *src = static_cast<byte *>(array->m_array) + (type.size() * array->m_entity_to_index.get(entity));
								type.copy_constructor(dest, src);
							}
						}
						else
						{
						}
					}
				}
			}
		}
	}

	void init_windows(EditorRuntime *runtime, Project *project, EditorWindowState *state)
	{
		state->key_listener = Platform::register_key_callback(engine()->window, state, on_key_press);
		resource_window_init(project, state);
		state->texture = Platform::init_texture(runtime->upload_context, EditorResources::TEXTURE_PNG_WIDTH, EditorResources::TEXTURE_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->texture, EditorResources::GET_TEXTURE_PNG(), EditorResources::TEXTURE_PNG_LEN);
		state->cpp_source = Platform::init_texture(runtime->upload_context, EditorResources::CPP_PNG_WIDTH, EditorResources::CPP_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->cpp_source, EditorResources::GET_CPP_PNG(), EditorResources::CPP_PNG_LEN);
		state->shader = Platform::init_texture(runtime->upload_context, EditorResources::SHADER_PNG_WIDTH, EditorResources::SHADER_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->shader, EditorResources::GET_SHADER_PNG(), EditorResources::SHADER_PNG_LEN);
		state->file = Platform::init_texture(runtime->upload_context, EditorResources::FILE_PNG_WIDTH, EditorResources::FILE_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->file, EditorResources::GET_FILE_PNG(), EditorResources::FILE_PNG_LEN);
		state->folder = Platform::init_texture(runtime->upload_context, EditorResources::FOLDER_PNG_WIDTH, EditorResources::FOLDER_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->folder, EditorResources::GET_FOLDER_PNG(), EditorResources::FOLDER_PNG_LEN);
		state->mesh = Platform::init_texture(runtime->upload_context, EditorResources::MESH_PNG_WIDTH, EditorResources::MESH_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->mesh, EditorResources::GET_MESH_PNG(), EditorResources::MESH_PNG_LEN);
	}

	void update_windows(EditorWindowState *state, f64 dt) { scene_window_update(state, dt); }

	void render_windows(Platform::CmdBuffer *cmd, Project *project, EditorWindowState *state, EditorRuntime *runtime, f64 dt)
	{
		Platform::imgui_begin_frame(cmd, runtime->imgui_c);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
										ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("VultrDockspace", &state->dockspace_open, window_flags);
		ImGui::PopStyleVar(3);

		ImGui::Text("%.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save Scene"))
				{
					serialize_current_scene(state);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Run"))
			{
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (ImGui::BeginPopupModal("Import"))
		{
			ImGui::Text("Importing resources!");
			ImGui::ProgressBar(static_cast<f32>(state->resource_import_state.progress) / static_cast<f32>(state->resource_import_state.total));
			if (!state->resource_import_state.importing)
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		else
		{
			if (state->resource_import_state.importing)
			{
				ImGui::OpenPopup("Import");
			}
		}

		if (ImGui::BeginPopupModal("Import Error"))
		{
			ImGui::Text("Failed to import resources!");
			ImGui::Text("%s", state->resource_import_state.import_error.value().message.c_str());
			if (ImGui::Button("Ok"))
			{
				state->resource_import_state.import_error = None;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		else
		{
			if (!state->resource_import_state.importing && state->resource_import_state.import_error)
			{
				ImGui::OpenPopup("Import Error");
			}
		}

		auto dockspace = ImGui::GetID("HUB_DockSpace");
		ImGui::DockSpace(dockspace, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoResize);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		scene_window_draw(state, runtime);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow();

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		entity_hierarchy_window_draw(project, state);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		component_inspector_window_draw(project, state);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		resource_browser_window_draw(project, state);

		ImGui::End();

		Platform::imgui_end_frame(cmd, runtime->imgui_c);
	}

	void destroy_windows(EditorWindowState *state)
	{
		auto *c = engine()->context;
		Platform::destroy_texture(c, state->texture);
		Platform::destroy_texture(c, state->cpp_source);
		Platform::destroy_texture(c, state->shader);
		Platform::destroy_texture(c, state->file);
		Platform::destroy_texture(c, state->folder);
		Platform::destroy_texture(c, state->mesh);
	}

	static ErrorOr<void> import_resource_file(const Project *project, const Path &local_src, const Path &full_src, atomic_u32 *progress)
	{
		switch (get_resource_type(local_src))
		{
			case ResourceType::MESH:
			{
				auto [vertex_out, index_out] = get_mesh_resource(project, local_src);
				if (needs_reimport(full_src, vertex_out) || needs_reimport(full_src, index_out))
				{
					ftouch(full_src);
					printf("Importing mesh %s\n", full_src.c_str());
					TRY_UNWRAP(auto mesh, Platform::import_mesh_file(full_src));
					TRY(Platform::export_mesh(vertex_out, index_out, &mesh));
					Platform::free_imported_mesh(&mesh);
					(*progress)++;
				}
				break;
			}
			case ResourceType::SHADER:
			{
				auto [vertex_out, fragment_out] = get_shader_resource(project, local_src);
				if (needs_reimport(full_src, vertex_out) || needs_reimport(full_src, fragment_out))
				{
					ftouch(full_src);
					String src{};
					TRY(try_fread_all(full_src, &src));
					TRY_UNWRAP(auto compiled, Platform::try_compile_shader(src));
					TRY(try_fwrite_all(vertex_out, compiled.vert_src, StreamWriteMode::OVERWRITE));
					TRY(try_fwrite_all(fragment_out, compiled.frag_src, StreamWriteMode::OVERWRITE));
					(*progress)++;
				}
				break;
			}
			case ResourceType::TEXTURE:
			{
				auto out = get_texture_resource(project, local_src);
				if (needs_reimport(full_src, out))
				{
					ftouch(full_src);
					Buffer imported{};
					TRY(Platform::import_texture_file(full_src, &imported));
					TRY(try_fwrite_all(out, imported, StreamWriteMode::OVERWRITE));
					(*progress)++;
				}
				break;
			}
			default:
			{
				auto out = project->build_resource_dir / local_src;
				if (needs_reimport(full_src, out))
				{
					ftouch(full_src);
					Buffer src{};
					TRY(try_fread_all(full_src, &src));
					TRY(try_fwrite_all(out, src, StreamWriteMode::OVERWRITE));
					(*progress)++;
				}
				break;
			}
		}
		return Success;
	}

	static ErrorOr<void> import_dir(const Project *project, const Path &in_dir, atomic_u32 *progress)
	{
		for (auto entry : DirectoryIterator(in_dir))
		{
			auto local = local_resource_file(project, entry).value();
			if (entry.is_directory())
			{
				TRY(makedir(project->build_resource_dir / local));
				TRY(import_dir(project, entry, progress));
			}
			else
			{
				TRY(import_resource_file(project, local, entry, progress))
			}
		}
		return Success;
	}

	static bool needs_reimport(const Project *project, const Path &path)
	{
		auto local = local_resource_file(project, path).value();
		switch (get_resource_type(path))
		{
			case ResourceType::TEXTURE:
			{
				auto out = get_texture_resource(project, local);
				return needs_reimport(path, out);
			}
			case ResourceType::SHADER:
			{
				auto [vertex_out, fragment_out] = get_shader_resource(project, local);
				return (needs_reimport(path, vertex_out) || needs_reimport(path, fragment_out));
			}
			case ResourceType::MATERIAL:
			{
				auto out = get_material_resource(project, local);
				return needs_reimport(path, out);
			}
			case ResourceType::MESH:
			{
				auto [vertex_out, index_out] = get_mesh_resource(project, local);
				return (needs_reimport(path, vertex_out) || needs_reimport(path, index_out));
			}
			default:
			{
				return needs_reimport(path, project->build_resource_dir / local);
			}
		}
	}

	static u32 count_resource_imports(const Project *project, const Path &dir)
	{
		u32 count = 0;
		for (auto entry : DirectoryIterator(dir))
		{
			if (entry.is_file())
			{
				if (needs_reimport(project, entry))
					count++;
			}
			else
			{
				count += count_resource_imports(project, entry);
			}
		}
		return count;
	}

	static void resource_import_thread(Project *project, EditorWindowState *state)
	{
		ASSERT(!state->resource_import_state.importing, "Already importing!");
		state->resource_import_state.importing = true;
		auto res                               = makedir(project->build_resource_dir);
		if (!res.is_error())
		{
			state->resource_import_state.total = count_resource_imports(project, project->resource_dir);
			auto res                           = import_dir(project, project->resource_dir, &state->resource_import_state.progress);
			if (!res.is_error())
			{
				state->resource_import_state.import_error = None;
			}
			else
			{
				state->resource_import_state.import_error = res.get_error();
			}
		}
		else
		{
			state->resource_import_state.import_error = res.get_error();
		}
		state->resource_import_state.importing     = false;
		state->resource_browser_state.need_refresh = true;
	}

	void begin_resource_import(Project *project, EditorWindowState *state)
	{
		Platform::Thread thread(resource_import_thread, project, state);
		thread.detach();
	}
} // namespace Vultr
