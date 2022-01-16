#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"
#include <glad/glad.h>

int Vultr::vultr_main(Platform::EntryArgs *args)
{
	g_game_memory = init_game_memory();

	auto *window  = Platform::open_window(Platform::DisplayMode::WINDOWED, nullptr, "Vultr Game Engine", true);

	if check (Vultr::load_game("/home/brandon/Dev/VultrSandbox/build/libVultrDemo.so"), auto project, auto err)
	{
		project.init();

		while (!Platform::window_should_close(window))
		{
			//			glClearColor(1, 1, 1, 1);
			//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//
			project.update();
			Platform::swap_buffers(window);
			Platform::poll_events(window);
		}
		Platform::close_window(window);

		linear_free(g_game_memory->persistent_storage);

		return 0;
	}
	else
	{
		fprintf(stderr, "Failed to load project file: %s", (str)err.message);
		return 1;
	}
}
