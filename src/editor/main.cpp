#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"
#include <glm/ext/quaternion_trigonometric.hpp>
#include <imgui/imgui.h>
#include <core/systems/render_system.h>
#include <core/systems/resource_system.h>
#include <filesystem/filestream.h>

int Vultr::vultr_main(Platform::EntryArgs *args)
{
	Vultr::init();
	int return_code = 0;
	if check (pwd(), auto cwd, auto cwd_err)
	{
		auto resource_dir = cwd / "res/";
		auto build_dir    = cwd / "build/";
		if (!exists(resource_dir))
			THROW("Resource directory does not exist!");

		if (!exists(build_dir))
			THROW("Build directory does not exist!");

		auto dll = build_dir / VULTR_GAMEPLAY_NAME;
		printf("Opening %s\n", dll.m_path.c_str());

		if check (Vultr::load_game(dll), auto project, auto err)
		{
			Vultr::open_borderless_windowed("Vultr Game Engine");

			project.init();

			auto *resource_system = ResourceSystem::init(resource_dir);
			auto *render_system   = RenderSystem::init(build_dir);

			create_entity(Mesh{.source = Path("cube.fbx")}, Transform{}, Material{});
			auto camera_ent       = create_entity(Camera{}, Transform{.position = Vec3(0, 0, 10)});
			auto light_ent        = create_entity(Transform{}, DirectionalLight{});
			auto *imgui_c         = Platform::init_imgui(engine()->window, resource_system->upload_context);
			bool show_demo_window = true;

			auto *fullscreen_quad = Platform::init_fullscreen_quad(resource_system->upload_context);
			Buffer buf;
			fread_all(build_dir / "shaders/fullscreen_vert.spv", &buf);
			CHECK_UNWRAP(auto *example_vert, Platform::try_load_shader(engine()->context, buf, Platform::ShaderType::VERT));

			buf.clear();
			fread_all(build_dir / "shaders/fullscreen_frag.spv", &buf);
			CHECK_UNWRAP(auto *example_frag, Platform::try_load_shader(engine()->context, buf, Platform::ShaderType::FRAG));

			auto *descriptor_layout = Platform::init_descriptor_layout<Platform::TextureBinding>(engine()->context, 1);
			Platform::register_descriptor_layout(engine()->context, descriptor_layout);

			Platform::GraphicsPipelineInfo info{
				.vert               = example_vert,
				.frag               = example_frag,
				.descriptor_layouts = Vector({descriptor_layout}),
			};

			auto *pipeline = Platform::init_pipeline(engine()->context, info);

			Platform::destroy_shader(engine()->context, example_vert);
			Platform::destroy_shader(engine()->context, example_frag);

			CHECK_UNWRAP(auto *texture, Platform::load_texture_file(resource_system->upload_context, build_dir / "textures/human_trollface.jpg"));

			while (!Platform::window_should_close(engine()->window))
			{
				Platform::poll_events(engine()->window);
				auto dt = Platform::update_window(engine()->window);

				if (Platform::mouse_down(engine()->window, Platform::Input::MouseButton::MOUSE_RIGHT))
				{
					Platform::lock_cursor(engine()->window);

					static constexpr f32 speed = 2;
					static constexpr f32 sens  = 100000;
					f32 delta                  = speed * dt;
					auto &transform            = get_component<Transform>(camera_ent);

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

					auto mouse_delta                             = Platform::get_mouse_delta(engine()->window);

					f64 aspect_ratio                             = (f64)Platform::get_window_width(engine()->window) / (f64)Platform::get_window_width(engine()->window);
					Quat rotation_horiz                          = glm::angleAxis(f32(sens * dt * -mouse_delta.x * aspect_ratio), Vec3(0, 1, 0));
					Quat rotation_vert                           = glm::angleAxis(f32(sens * dt * mouse_delta.y), right(transform));
					transform.rotation                           = rotation_horiz * rotation_vert * transform.rotation;
					get_component<Transform>(light_ent).rotation = transform.rotation;
				}
				else
				{
					Platform::unlock_cursor(engine()->window);
				}

				if check (Platform::begin_cmd_buffer(engine()->window), auto *cmd, auto _)
				{
					RenderSystem::update(cmd, render_system, resource_system);
					ResourceSystem::update(resource_system);

					Platform::begin_window_framebuffer(cmd);

					Platform::imgui_begin_frame(cmd, imgui_c);

					ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
													ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
					ImGuiViewport *viewport = ImGui::GetMainViewport();
					ImGui::SetNextWindowPos(viewport->Pos);
					ImGui::SetNextWindowSize(viewport->Size);
					ImGui::SetNextWindowViewport(viewport->ID);

					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

					static bool dockspace_open = true;
					ImGui::Begin("DockSpace Demo", &dockspace_open, window_flags);
					ImGui::PopStyleVar(3);
					ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
					auto dockspace = ImGui::GetID("HUB_DockSpace");
					ImGui::DockSpace(dockspace, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoResize);

					if (show_demo_window)
					{
						ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
						ImGui::ShowDemoWindow(&show_demo_window);
					}

					ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);

					ImGui::Begin("Game");
					ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
					auto output_texture        = Platform::imgui_get_texture_id(Platform::get_attachment_texture(render_system->output_framebuffer, 0));
					ImGui::Image(output_texture, viewport_panel_size);
					ImGui::End();

					ImGui::End();

					Platform::imgui_end_frame(cmd, imgui_c);

					Platform::end_framebuffer(cmd);

					Platform::end_cmd_buffer(cmd);
				}
				else
				{
					RenderSystem::reinitialize(render_system);
				}
				project.update();
			}
			Platform::wait_idle(engine()->context);

			Platform::destroy_texture(engine()->context, texture);
			Platform::destroy_descriptor_layout(engine()->context, descriptor_layout);
			Platform::destroy_pipeline(engine()->context, pipeline);
			Platform::destroy_mesh(resource_system->upload_context, fullscreen_quad);

			Platform::destroy_imgui(engine()->context, imgui_c);
			ResourceSystem::destroy(resource_system);
			RenderSystem::destroy(render_system);
			Platform::close_window(engine()->window);
		}
		else
		{
			fprintf(stderr, "Failed to load project file: %s\n", (str)err.message);
			return_code = 1;
		}
	}
	else
	{
		fprintf(stderr, "Failed to get current working directory: %s\n", cwd_err.message.c_str());
		return_code = 1;
	}

	Vultr::destroy();

	return return_code;
}
