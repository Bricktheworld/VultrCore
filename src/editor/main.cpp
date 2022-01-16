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

		auto *c = Platform::get_render_context(window);
		while (!Platform::window_should_close(window))
		{
			Platform::poll_events(window);
			project.update();
			Platform::draw_frame(window, c);
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
