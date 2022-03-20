#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"
#include "windows/windows.h"
#include "editor/runtime/runtime.h"
#include <core/systems/render_system.h>
#include <core/systems/resource_system.h>
#include <filesystem/filestream.h>
#include <vultr_resource_manager.h>

[[noreturn]] static void texture_loader_thread()
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Texture *>();
	while (true)
	{
		printf("Waiting for another texture to load...\n");
		auto [resource, path] = allocator->wait_pop_load_queue();
		printf("Loading texture %u, from path %s\n", resource, path.c_str());
		if (!allocator->add_loaded_resource(resource, reinterpret_cast<Vultr::Platform::Texture *>(0xFFFFFFFFF)).has_value())
		{
			printf("Freeing unnecessary load!\n");
		}
	}
}

[[noreturn]] static void texture_free_thread()
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Texture *>();
	while (true)
	{
		printf("Waiting for another texture to free...\n");
		auto *texture = allocator->wait_pop_free_queue();
		printf("Freeing texture data at location %p\n", texture);
	}
}

int Vultr::vultr_main(Platform::EntryArgs *args)
{
	Vultr::init();
	int return_code = 0;
	if check (pwd(), auto cwd, auto cwd_err)
	{
		auto resource_dir = cwd / "res/";
		auto build_dir    = cwd / "build/";
		ASSERT(exists(resource_dir), "Resource directory does not exist!");
		ASSERT(exists(build_dir), "Build directory does not exist!");

		if check (Vultr::load_game(build_dir, resource_dir), auto project, auto err)
		{
			Vultr::open_borderless_windowed("Vultr Game Engine");
			{
				auto *upload_context = Platform::init_upload_context(engine()->context);
				CHECK(Vultr::import_resource_dir(&project));
				Platform::destroy_upload_context(upload_context);
			}

			Vultr::init_resource_allocators(resource_dir);

			Platform::Thread loading_thread(texture_loader_thread);
			loading_thread.detach();
			Platform::Thread freeing_thread(texture_free_thread);
			freeing_thread.detach();

			EditorRuntime runtime{};
			runtime.resource_system = ResourceSystem::init(resource_dir);
			runtime.render_system   = RenderSystem::init(build_dir);
			runtime.imgui_c         = Platform::init_imgui(engine()->window, runtime.resource_system->upload_context);

			EditorWindowState state{};

			create_entity(Mesh{.source = Path("cube.fbx")}, Transform{}, Material{});
			auto light_ent = create_entity(Transform{}, DirectionalLight{});

			{
				auto resource = Resource<Platform::Texture *>("cube.fbx");
				{
					auto reference = resource;
				}
				sleep(1);
			}

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
