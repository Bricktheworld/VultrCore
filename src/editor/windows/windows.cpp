#include "windows.h"
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>

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
			// TODO(Brandon): Fix this in the coordinate system.
			if (Platform::key_down(engine()->window, Platform::Input::KEY_E))
				transform.position -= Vec3(0, 1, 0) * delta;
			if (Platform::key_down(engine()->window, Platform::Input::KEY_Q))
				transform.position += Vec3(0, 1, 0) * delta;

			auto mouse_delta    = Platform::get_mouse_delta(engine()->window);

			f64 aspect_ratio    = (f64)Platform::get_window_width(engine()->window) / (f64)Platform::get_window_width(engine()->window);
			Quat rotation_horiz = glm::angleAxis(f32(sens * dt * -mouse_delta.x * aspect_ratio), Vec3(0, 1, 0));
			Quat rotation_vert  = glm::angleAxis(f32(sens * dt * mouse_delta.y), right(transform));
			transform.rotation  = rotation_horiz * rotation_vert * transform.rotation;
		}
		else
		{
			Platform::unlock_cursor(engine()->window);
		}
	}

	void scene_window_draw(EditorRuntime *runtime)
	{
		ImGui::Begin("Game");
		ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
		auto output_texture        = Platform::imgui_get_texture_id(Platform::get_attachment_texture(runtime->render_system->output_framebuffer, 0));
		ImGui::Image(output_texture, viewport_panel_size);
		ImGui::End();
	}
	void entity_hierarchy_window_draw(EditorWindowState *state)
	{
		ImGui::Begin("Hierarchy");
		for (Entity entity = 1; entity < MAX_ENTITIES; entity++)
		{
			if (!entity_exists(entity))
				continue;

			if (ImGui::Selectable("Entity " + String(entity), state->selected_entity == entity))
				state->selected_entity = entity;
		}
		ImGui::End();
	}
	void component_inspector_window_draw(EditorWindowState *state)
	{
		ImGui::Begin("Inspector");
		if (state->selected_entity)
		{
			auto info = world()->component_manager.get_component_information(state->selected_entity.value());
			for (auto [component_name, members] : info)
			{
				if (ImGui::CollapsingHeader(component_name))
				{
					ImGui::PushID(component_name);
					for (auto member : members)
					{
						switch (member.type)
						{
							case PrimitiveType::U8:
								ImGui::DragScalar(member.name, ImGuiDataType_U8, member.addr);
								break;
							case PrimitiveType::U16:
								ImGui::DragScalar(member.name, ImGuiDataType_U16, member.addr);
								break;
							case PrimitiveType::U32:
								ImGui::DragScalar(member.name, ImGuiDataType_U32, member.addr);
								break;
							case PrimitiveType::U64:
								ImGui::DragScalar(member.name, ImGuiDataType_U64, member.addr);
								break;
							case PrimitiveType::S8:
								ImGui::DragScalar(member.name, ImGuiDataType_S8, member.addr);
								break;
							case PrimitiveType::S16:
								ImGui::DragScalar(member.name, ImGuiDataType_S16, member.addr);
								break;
							case PrimitiveType::S32:
								ImGui::DragScalar(member.name, ImGuiDataType_S32, member.addr);
								break;
							case PrimitiveType::S64:
								ImGui::DragScalar(member.name, ImGuiDataType_S64, member.addr);
								break;
							case PrimitiveType::F32:
								ImGui::DragScalar(member.name, ImGuiDataType_Float, member.addr);
								break;
							case PrimitiveType::F64:
								ImGui::DragScalar(member.name, ImGuiDataType_Double, member.addr);
								break;
							case PrimitiveType::CHAR:
								ImGui::Text("%s Char %c", member.name.c_str(), *static_cast<char *>(member.addr));
								break;
							case PrimitiveType::BYTE:
								ImGui::Text("%s Byte %u", member.name.c_str(), *static_cast<byte *>(member.addr));
								break;
							case PrimitiveType::BOOL:
								ImGui::Checkbox(member.name, static_cast<bool *>(member.addr));
								break;
							case PrimitiveType::STRING_VIEW:
								ImGui::Text("%s String %s", member.name.c_str(), static_cast<StringView *>(member.addr)->c_str());
								break;
							case PrimitiveType::VOID_PTR:
								ImGui::Text("%s void * %p", member.name.c_str(), member.addr);
								break;
							case PrimitiveType::VEC2:
							{
								Vec2 *vec2 = static_cast<Vec2 *>(member.addr);
								ImGui::Text("%s", member.name.c_str());
								ImGui::SameLine();

								ImGui::PushID((member.name + ".x").c_str());
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &vec2->x, 0.02f);
								ImGui::SameLine();
								ImGui::PopID();

								ImGui::PushID((member.name + ".y").c_str());
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &vec2->y, 0.02f);
								ImGui::SameLine();
								ImGui::PopID();
								break;
							}
							case PrimitiveType::VEC3:
							{
								Vec3 *vec3 = static_cast<Vec3 *>(member.addr);
								ImGui::PushID((member.name + ".x").c_str());
								ImGui::Text("%s", member.name.c_str());
								ImGui::SameLine();
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &vec3->x, 0.02f);
								ImGui::SameLine();
								ImGui::PopID();

								ImGui::PushID((member.name + ".y").c_str());
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &vec3->y, 0.02f);
								ImGui::SameLine();
								ImGui::PopID();

								ImGui::PushID((member.name + ".z").c_str());
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &vec3->z, 0.02f);
								ImGui::PopID();
								break;
							}
							case PrimitiveType::VEC4:
							{
								Vec4 *vec4 = static_cast<Vec4 *>(member.addr);
								ImGui::PushID((member.name + ".x").c_str());
								ImGui::Text("%s", member.name.c_str());
								ImGui::SameLine();
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &vec4->x, 0.02f);
								ImGui::SameLine();
								ImGui::PopID();

								ImGui::PushID((member.name + ".y").c_str());
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &vec4->y, 0.02f);
								ImGui::SameLine();
								ImGui::PopID();

								ImGui::PushID((member.name + ".z").c_str());
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &vec4->z, 0.02f);
								ImGui::SameLine();
								ImGui::PopID();

								ImGui::PushID((member.name + ".w").c_str());
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &vec4->w, 0.02f);
								ImGui::PopID();
								break;
							}
							case PrimitiveType::COLOR:
							{
								f32 *val = glm::value_ptr(*static_cast<Vec4 *>(member.addr));
								ImGui::ColorEdit4(member.name, val);
								break;
							}
							case PrimitiveType::QUAT:
							{
								Quat *quat = static_cast<Quat *>(member.addr);
								ImGui::PushID((member.name + ".x").c_str());
								ImGui::Text("%s", member.name.c_str());
								ImGui::SameLine();
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &quat->x, 0.02f);
								ImGui::SameLine();
								ImGui::PopID();

								ImGui::PushID((member.name + ".y").c_str());
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &quat->y, 0.02f);
								ImGui::SameLine();
								ImGui::PopID();

								ImGui::PushID((member.name + ".z").c_str());
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &quat->z, 0.02f);
								ImGui::SameLine();
								ImGui::PopID();

								ImGui::PushID((member.name + ".w").c_str());
								ImGui::SetNextItemWidth(150);
								ImGui::DragFloat("", &quat->w, 0.02f);
								ImGui::PopID();
								break;
							}
							case PrimitiveType::PATH:
								break;
							case PrimitiveType::OPTIONAL_PATH:
								ImGui::Text("OPTIONAL_PATH");
								break;
							case PrimitiveType::OTHER:
								break;
							case PrimitiveType::STRING:
								break;
							case PrimitiveType::RESOURCE:
								ImGui::Text("RESOURCE");
								break;
						}
					}
					if (ImGui::Button("Remove"))
					{
					}

					ImGui::PopID();
				}
			}
		}
		ImGui::End();
	}

	void update_windows(EditorWindowState *state, f64 dt) { scene_window_update(state, dt); }

	void render_windows(Platform::CmdBuffer *cmd, EditorWindowState *state, EditorRuntime *runtime, f64 dt)
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
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		auto dockspace = ImGui::GetID("HUB_DockSpace");
		ImGui::DockSpace(dockspace, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoResize);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		scene_window_draw(runtime);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		entity_hierarchy_window_draw(state);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		component_inspector_window_draw(state);

		ImGui::End();

		Platform::imgui_end_frame(cmd, runtime->imgui_c);
	}
} // namespace Vultr
