#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"
#include "windows/windows.h"
#include "editor/runtime/runtime.h"
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

			EditorRuntime runtime{};
			runtime.resource_system = ResourceSystem::init(resource_dir);
			runtime.render_system   = RenderSystem::init(build_dir);
			runtime.imgui_c         = Platform::init_imgui(engine()->window, runtime.resource_system->upload_context);

			EditorWindowState state{};

			create_entity(Mesh{.source = Path("cube.fbx")}, Transform{}, Material{});
			auto light_ent = create_entity(Transform{}, DirectionalLight{});

			project.init();

			while (!Platform::window_should_close(engine()->window))
			{
				Platform::poll_events(engine()->window);
				auto dt = Platform::update_window(engine()->window);

				update_windows(&state, dt);

				if check (Platform::begin_cmd_buffer(engine()->window), auto *cmd, auto _)
				{
					RenderSystem::update(state.editor_camera, state.editor_camera_transform, cmd, runtime.render_system, runtime.resource_system);
					ResourceSystem::update(runtime.resource_system);

					Platform::begin_window_framebuffer(cmd);
					render_windows(cmd, &state, &runtime, dt);
					Platform::end_framebuffer(cmd);

					Platform::end_cmd_buffer(cmd);
				}
				else
				{
					RenderSystem::reinitialize(runtime.render_system);
				}
				project.update(nullptr);
			}
			Platform::wait_idle(engine()->context);

			Platform::destroy_imgui(engine()->context, runtime.imgui_c);
			ResourceSystem::destroy(runtime.resource_system);
			RenderSystem::destroy(runtime.render_system);
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
