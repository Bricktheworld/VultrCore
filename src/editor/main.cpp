#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"
#include "glm/ext/quaternion_trigonometric.hpp"
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

			auto *render_system   = RenderSystem::init();
			auto *resource_system = ResourceSystem::init(render_system, resource_dir, build_dir);

			auto some_ent         = create_entity(Mesh{.source = Path("cube.fbx")}, Transform{}, Material{});
			auto camera_ent       = create_entity(Camera{}, Transform{.position = Vec3(0, 0, 10)});
			auto *imgui_c         = Platform::init_imgui(engine()->window, resource_system->upload_context);
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

				RenderSystem::update(render_system, resource_system);
				ResourceSystem::update(resource_system);
				project.update();
			}
			Platform::wait_idle(engine()->context);
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
