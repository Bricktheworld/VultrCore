#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"

int Vultr::vultr_main(Platform::EntryArgs *args)
{
	Vultr::init();
	auto *window = Platform::open_window(Platform::DisplayMode::WINDOWED, nullptr, "Vultr Game Engine", true);

	if check (Vultr::load_game("/home/brandon/Dev/VultrSandbox/build/libVultrDemo.so"), auto project, auto err)
	{
		project.init();

		while (!Platform::window_should_close(window))
		{
			Platform::poll_events(window);
			project.update();
			Platform::update_window(window);
		}
		Platform::close_window(window);
		Vultr::destroy();

		return 0;
	}
	else
	{
		fprintf(stderr, "Failed to load project file: %s", (str)err.message);
		return 1;
	}
}
