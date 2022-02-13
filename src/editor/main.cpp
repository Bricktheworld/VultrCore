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

			auto *render_system = RenderSystem::init();
			while (!Platform::window_should_close(engine()->window))
			{
				Platform::poll_events(engine()->window);
				auto dt = Platform::update_window(engine()->window);
				RenderSystem::update(render_system);
				project.update();
			}
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
