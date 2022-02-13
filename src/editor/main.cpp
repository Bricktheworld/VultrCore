#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"
#include <core/systems/render_system.h>

int Vultr::vultr_main(Platform::EntryArgs *args)
{
	Vultr::init();
	Vultr::open_borderless_windowed("Vultr Game Engine");

	if check (Vultr::load_game("/home/brandon/Dev/VultrSandbox/build/libVultrDemo.so"), auto project, auto err)
	{
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
		fprintf(stderr, "Failed to load project file: %s", (str)err.message);
		return 1;
	}
}
