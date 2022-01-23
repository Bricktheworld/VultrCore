#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"
#include <core/components/transform.h>
#include <core/components/mesh.h>
#include "renderer/material.h"
#include <ecs/world.h>

int Vultr::vultr_main(Platform::EntryArgs *args)
{
	g_game_memory = init_game_memory();

	auto *window  = Platform::open_window(Platform::DisplayMode::WINDOWED, nullptr, "Vultr Game Engine", true);

	ComponentManager<Mesh, Transform, Material> component_manager{};

	if check (Vultr::load_game("/home/brandon/Dev/VultrSandbox/build/libVultrDemo.so"), auto project, auto err)
	{
		project.init();
		{
			auto &mesh         = component_manager.get_component<Mesh>(0);
			mesh.source        = Path("Some path");
			auto &transform    = component_manager.get_component<Transform>(0);
			transform.position = Vec3(20, 20, 30);
		}
		{
			auto [mesh, transform] = component_manager.get_components<Mesh, Transform>(0);
			ASSERT(transform.position == Vec3(20, 20, 30), "Transform not correct!");
			ASSERT(mesh.source.has_value() && mesh.source.value().m_path == "Some path", "Mesh not correct!");
		}

		while (!Platform::window_should_close(window))
		{
			Platform::poll_events(window);
			project.update();
			Platform::update_window(window);
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
