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
		{
			create_entity<Mesh, Transform>({}, {});
			create_entity<Mesh, Transform, Material>({}, {}, {});
			{
				auto ent3 = create_entity();
				add_component<Mesh>(ent3, {});

				auto ent4 = create_entity();
				add_component<Transform>(ent4, {});
				for (auto [entity, mesh, transform] : get_entities<Mesh, Transform>())
				{
					transform.position.x += 20;
					printf("%u Has a mesh {%f} and transform\n", entity, transform.position.x);
				}
			}
			//			auto &mesh         = world.component_manager.get_component<Mesh>(ent);
			//			mesh.source        = Path("Some path");
			//			auto &transform    = component_manager.get_component<Transform>(0);
			//			transform.position = Vec3(20, 20, 30);
		}

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
