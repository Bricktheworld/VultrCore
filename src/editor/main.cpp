#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"
#include "windows/windows.h"
#include "editor/runtime/runtime.h"
#include <core/systems/render_system.h>
#include <filesystem/filestream.h>
#include <vultr_resource_allocator.h>

[[noreturn]] static void mesh_loader_thread(Vultr::Platform::UploadContext *c, const Vultr::Path &resource_dir)
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Mesh *>();
	while (true)
	{
		printf("Waiting for another mesh to load...\n");
		auto [resource, path] = allocator->wait_pop_load_queue();
		printf("Loading mesh %u, from path %s\n", resource, path.c_str());

		Vultr::Buffer vertex_buffer;
		Vultr::fread_all(resource_dir / (path.string() + ".vertex"), &vertex_buffer);

		Vultr::Buffer index_buffer;
		Vultr::fread_all(resource_dir / (path.string() + ".index"), &index_buffer);

		auto *mesh = Vultr::Platform::load_mesh_memory(c, vertex_buffer, index_buffer);

		if (!allocator->add_loaded_resource(resource, mesh).has_value())
		{
			printf("Freeing unnecessary load!\n");
			Vultr::Platform::destroy_mesh(c, mesh);
		}
	}
}

[[noreturn]] static void mesh_free_thread(Vultr::Platform::UploadContext *c)
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Mesh *>();
	while (true)
	{
		printf("Waiting for another mesh to free...\n");
		auto *mesh = allocator->wait_pop_free_queue();
		printf("Freeing mesh data at location %p\n", mesh);
		Vultr::Platform::destroy_mesh(c, mesh);
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

			Vultr::init_resource_allocators();

			auto *c = Vultr::Platform::init_upload_context(Vultr::engine()->context);
			Platform::Thread loading_thread(mesh_loader_thread, c, build_dir / "res");
			loading_thread.detach();
			Platform::Thread freeing_thread(mesh_free_thread, c);
			freeing_thread.detach();

			EditorRuntime runtime{};
			runtime.render_system  = RenderSystem::init(build_dir);
			runtime.upload_context = Platform::init_upload_context(engine()->context);
			runtime.imgui_c        = Platform::init_imgui(engine()->window, runtime.upload_context);

			EditorWindowState state{};

			create_entity(Mesh{.source = Resource<Platform::Mesh *>("subdir/sphere.fbx")}, Transform{}, Material{});
			auto light_ent = create_entity(Transform{}, DirectionalLight{});

			project.init();

			while (!Platform::window_should_close(engine()->window))
			{
				Platform::poll_events(engine()->window);
				auto dt = Platform::update_window(engine()->window);

				update_windows(&state, dt);

				if check (Platform::begin_cmd_buffer(engine()->window), auto *cmd, auto _)
				{
					RenderSystem::update(state.editor_camera, state.editor_camera_transform, cmd, runtime.render_system);

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
