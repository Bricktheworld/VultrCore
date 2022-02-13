#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"
#include <core/systems/render_system.h>

int Vultr::vultr_main(Platform::EntryArgs *args)
{
	Vultr::init();
	if check (pwd(), auto cwd, auto cwd_err)
	{
		auto dll = cwd / VULTR_GAMEPLAY_LIBRARY_LOCATION;
		printf("Opening %s\n", dll.m_path.c_str());

		if check (Vultr::load_game(dll), auto project, auto err)
		{
			Vultr::open_borderless_windowed("Vultr Game Engine");

			project.init();

			auto resource_dir = cwd / "res/";
			if (!exists(resource_dir))
				THROW("Resource directory does not exist!");

			auto *upload_context         = Platform::init_upload_context(engine()->context);
			Platform::Mesh *example_mesh = nullptr;
			auto cube_src                = resource_dir / "cube.fbx";
			if check (Platform::load_mesh_file(upload_context, cube_src), example_mesh, auto _)
			{
			}
			else
			{
				THROW("Failed to load %s", cube_src.m_path.c_str());
			}

			auto *render_system = RenderSystem::init();
			while (!Platform::window_should_close(engine()->window))
			{
				Platform::poll_events(engine()->window);
				auto dt = Platform::update_window(engine()->window);
				RenderSystem::update(render_system);
				project.update();
			}

			Platform::destroy_mesh(upload_context, example_mesh);
			Platform::destroy_upload_context(upload_context);
			Platform::close_window(engine()->window);
			Vultr::destroy();

			return 0;
		}
		else
		{
			fprintf(stderr, "Failed to load project file: %s\n", (str)err.message);
			return 1;
		}
	}
	else
	{
		fprintf(stderr, "Failed to get current working directory: %s\n", cwd_err.message.c_str());
		return 1;
	}
}
