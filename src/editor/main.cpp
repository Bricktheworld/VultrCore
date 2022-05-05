#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"
#include "windows/windows.h"
#include "editor/runtime/runtime.h"
#include <core/systems/render_system.h>
#include <filesystem/filestream.h>
#include <vultr_resource_allocator.h>
#include <platform/rendering/vulkan/texture.h>

// static void print_texture_resources(int line)
//{
//		auto *texture_allocator = Vultr::resource_allocator<Vultr::Platform::Texture *>();
//		for (auto &[_, info] : texture_allocator->resources)
//		{
//			if (info.load_state == Vultr::ResourceLoadState::LOADED)
//				printf("(%d) Sampler location for texture %p is %p\n", line, info.data, info.data->sampler);
//		}
// }

static void mesh_loader_thread(const Vultr::Path &resource_dir)
{
	auto *c         = Vultr::Platform::init_upload_context(Vultr::engine()->context);
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Mesh *>();
	while (true)
	{
		printf("Waiting for another mesh to load...\n");
		auto [resource, path, is_kill_request] = allocator->wait_pop_load_queue();

		if (is_kill_request)
		{
			printf("Shutting down mesh loading thread.\n");
			Vultr::Platform::destroy_upload_context(c);
			return;
		}

		printf("Loading mesh %u, from path %s\n", resource, path.c_str());

		Vultr::Buffer vertex_buffer;
		Vultr::fread_all(resource_dir / (path.string() + ".vertex"), &vertex_buffer);

		Vultr::Buffer index_buffer;
		Vultr::fread_all(resource_dir / (path.string() + ".index"), &index_buffer);

		auto *mesh = Vultr::Platform::load_mesh_memory(c, vertex_buffer, index_buffer);
		printf("Loaded mesh %p from path %s\n", mesh, path.c_str());

		if (allocator->add_loaded_resource(resource, mesh).is_error())
		{
			printf("Freeing unnecessary load!\n");
			Vultr::Platform::destroy_mesh(Vultr::engine()->context, mesh);
		}
	}
}

static void mesh_free_thread()
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Mesh *>();
	while (true)
	{
		printf("Waiting for another mesh to free...\n");
		auto *mesh = allocator->wait_pop_free_queue();

		if (mesh == (void *)-1)
		{
			printf("Shutting down mesh freeing thread.\n");
			return;
		}

		printf("Freeing mesh data at location %p\n", mesh);
		Vultr::Platform::destroy_mesh(Vultr::engine()->context, mesh);
	}
}

static void material_loader_thread(const Vultr::Path &resource_dir)
{
	auto *c         = Vultr::Platform::init_upload_context(Vultr::engine()->context);
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Material *>();
	while (true)
	{
		printf("Waiting for another material to load...\n");
		auto [resource, path, is_kill_request] = allocator->wait_pop_load_queue();

		if (is_kill_request)
		{
			printf("Shutting down material loading thread.\n");
			Vultr::Platform::destroy_upload_context(c);
			return;
		}

		printf("Loading material %u, from path %s\n", resource, path.c_str());

		Vultr::String material_src;
		Vultr::fread_all(resource_dir / path, &material_src);

		auto shader_path = Vultr::split(material_src, "\n")[0];
		auto shader      = Vultr::Resource<Vultr::Platform::Shader *>(Vultr::Path(shader_path));
		shader.wait_loaded();

		CHECK_UNWRAP(auto *mat, Vultr::Platform::try_load_material(c, shader, material_src));

		if (!allocator->add_loaded_resource(resource, mat).has_value())
		{
			printf("Freeing unnecessary load!\n");
			Vultr::Platform::destroy_material(Vultr::engine()->context, mat);
		}
	}
}

static void material_free_thread()
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Material *>();
	while (true)
	{
		printf("Waiting for another material to free...\n");
		auto *mat = allocator->wait_pop_free_queue();

		if (mat == (void *)-1)
		{
			printf("Shutting down material freeing thread.\n");
			return;
		}

		printf("Freeing material data at location %p\n", mat);
		Vultr::Platform::destroy_material(Vultr::engine()->context, mat);
	}
}

static void shader_loader_thread(const Vultr::Path &resource_dir)
{
	auto *c         = Vultr::Platform::init_upload_context(Vultr::engine()->context);
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Shader *>();
	while (true)
	{
		printf("Waiting for another shader to load...\n");

		auto [resource, path, is_kill_request] = allocator->wait_pop_load_queue();
		if (is_kill_request)
		{
			printf("Shutting down shader loading thread.\n");
			Vultr::Platform::destroy_upload_context(c);
			return;
		}

		printf("Loading shader %u, from path %s\n", resource, path.c_str());

		Vultr::Platform::CompiledShaderSrc shader_src{};
		Vultr::fread_all(resource_dir / (path.string() + ".vert_spv"), &shader_src.vert_src);

		Vultr::fread_all(resource_dir / (path.string() + ".frag_spv"), &shader_src.frag_src);

		CHECK_UNWRAP(auto reflection, Vultr::Platform::try_reflect_shader(shader_src));

		CHECK_UNWRAP(auto *shader, Vultr::Platform::try_load_shader(Vultr::engine()->context, shader_src, reflection));

		if (allocator->add_loaded_resource(resource, shader).is_error())
		{
			printf("Freeing unnecessary load!\n");
			Vultr::Platform::destroy_shader(Vultr::engine()->context, shader);
		}
	}
}

static void shader_free_thread()
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Shader *>();
	while (true)
	{
		printf("Waiting for another shader to free...\n");
		auto *shader = allocator->wait_pop_free_queue();

		if (shader == (void *)-1)
		{
			printf("Shutting down shader freeing thread.\n");
			return;
		}

		printf("Freeing shader data at location %p\n", shader);
		Vultr::Platform::destroy_shader(Vultr::engine()->context, shader);
	}
}

static void texture_loader_thread(const Vultr::Path &resource_dir)
{
	auto *c         = Vultr::Platform::init_upload_context(Vultr::engine()->context);
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Texture *>();
	while (true)
	{
		printf("Waiting for another texture to load...\n");
		auto [resource, path, is_kill_request] = allocator->wait_pop_load_queue();

		if (is_kill_request)
		{
			printf("Shutting down texture loading thread.\n");
			Vultr::Platform::destroy_upload_context(c);
			return;
		}

		printf("Loading texture %u, from path %s\n", resource, path.c_str());

		Vultr::Buffer src;
		Vultr::fread_all((resource_dir) / (path.string() + ".texture"), &src);
		auto *texture = Vultr::Platform::init_texture(c, *(f64 *)&src[0], *(f64 *)&src[sizeof(f64)], Vultr::Platform::TextureFormat::RGBA8);
		Vultr::Platform::fill_texture(c, texture, &src[sizeof(f64) * 2], src.size() - 2 * sizeof(f64));

		if (allocator->add_loaded_resource(resource, texture).is_error())
		{
			printf("Freeing unnecessary load!\n");
			Vultr::Platform::destroy_texture(Vultr::engine()->context, texture);
		}
	}
}

static void texture_free_thread()
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Texture *>();
	while (true)
	{
		printf("Waiting for another texture to free...\n");
		auto *texture = allocator->wait_pop_free_queue();

		if (texture == (void *)-1)
		{
			printf("Shutting down texture freeing thread.\n");
			return;
		}

		printf("Freeing texture data at location %p\n", texture);
		Vultr::Platform::destroy_texture(Vultr::engine()->context, texture);
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

			Platform::Thread mesh_loading_thread(mesh_loader_thread, build_dir / "res");
			mesh_loading_thread.detach();
			Platform::Thread mesh_freeing_thread(mesh_free_thread);
			mesh_freeing_thread.detach();
			Platform::Thread material_loading_thread(material_loader_thread, build_dir / "res");
			material_loading_thread.detach();
			Platform::Thread material_freeing_thread(material_free_thread);
			material_freeing_thread.detach();
			Platform::Thread shader_loading_thread(shader_loader_thread, build_dir / "res");
			shader_loading_thread.detach();
			Platform::Thread shader_freeing_thread(shader_free_thread);
			shader_freeing_thread.detach();
			Platform::Thread texture_loading_thread(texture_loader_thread, build_dir / "res");
			texture_loading_thread.detach();
			Platform::Thread texture_freeing_thread(texture_free_thread);
			texture_freeing_thread.detach();

			EditorRuntime runtime{};
			runtime.render_system  = RenderSystem::init();
			runtime.upload_context = Vultr::Platform::init_upload_context(Vultr::engine()->context);
			runtime.imgui_c        = Platform::init_imgui(engine()->window, runtime.upload_context);

			EditorWindowState state{};

			void *project_state = project.init();

			init_windows(&runtime, &project, &state);
			while (!Platform::window_should_close(engine()->window))
			{
				Platform::poll_events(engine()->window);
				auto dt = Platform::update_window(engine()->window);

				update_windows(&state, dt);

				project.update(project_state, dt);

				if check (Platform::begin_cmd_buffer(engine()->window), auto *cmd, auto _)
				{
					RenderSystem::update(state.editor_camera, state.editor_camera_transform, cmd, runtime.render_system);

					Platform::begin_window_framebuffer(cmd);
					render_windows(cmd, &project, &state, &runtime, dt);
					Platform::end_framebuffer(cmd);

					Platform::end_cmd_buffer(cmd);
				}
				else
				{
					RenderSystem::reinitialize(runtime.render_system);
				}
			}
			project.destroy(project_state);
			Platform::wait_idle(engine()->context);
			resource_allocator<Platform::Mesh *>()->kill_loading_threads();
			resource_allocator<Platform::Mesh *>()->kill_freeing_threads();
			resource_allocator<Platform::Material *>()->kill_loading_threads();
			resource_allocator<Platform::Material *>()->kill_freeing_threads();
			resource_allocator<Platform::Shader *>()->kill_loading_threads();
			resource_allocator<Platform::Shader *>()->kill_freeing_threads();
			resource_allocator<Platform::Texture *>()->kill_loading_threads();
			resource_allocator<Platform::Texture *>()->kill_freeing_threads();

			Platform::destroy_imgui(engine()->context, runtime.imgui_c);
			destroy_windows(&state);
			Platform::destroy_upload_context(runtime.upload_context);
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
