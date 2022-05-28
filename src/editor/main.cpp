#include <platform/platform.h>
#include <vultr.h>
#include "project/project.h"
#include "windows/windows.h"
#include "editor/runtime/runtime.h"
#include <core/systems/render_system.h>
#include <filesystem/filestream.h>
#include <vultr_resource_allocator.h>
#include <platform/rendering/vulkan/texture.h>
#include <platform/rendering/vulkan/vertex_buffer.h>
#include <platform/rendering/vulkan/index_buffer.h>

static void mesh_loader_thread(const Vultr::Project *project)
{
	auto *c         = Vultr::Platform::init_upload_context(Vultr::engine()->context);
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Mesh *>();
	while (true)
	{
		auto [resource, path, is_kill_request] = allocator->wait_pop_load_queue();

		if (is_kill_request)
		{
			Vultr::Platform::destroy_upload_context(c);
			return;
		}

		auto [vert, index] = Vultr::get_mesh_resource(project, path);

		Vultr::Buffer vertex_buffer;
		{
			auto res = Vultr::try_fread_all(vert, &vertex_buffer);

			if (res.is_error())
			{
				allocator->add_loaded_resource_error(resource, res.get_error());
				continue;
			}
		}

		Vultr::Buffer index_buffer;
		{
			auto res = Vultr::try_fread_all(index, &index_buffer);

			if (res.is_error())
			{
				allocator->add_loaded_resource_error(resource, res.get_error());
				continue;
			}
		}

		auto *mesh = Vultr::Platform::load_mesh_memory(c, vertex_buffer, index_buffer);

		if (allocator->add_loaded_resource(resource, mesh).is_error())
		{
			Vultr::Platform::destroy_mesh(Vultr::engine()->context, mesh);
		}
	}
}

static void mesh_free_thread()
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Mesh *>();
	while (true)
	{
		auto *mesh = allocator->wait_pop_free_queue();

		if (mesh == (void *)-1)
		{
			return;
		}

		Vultr::Platform::destroy_mesh(Vultr::engine()->context, mesh);
	}
}

static void material_loader_thread(const Vultr::Project *project)
{
	auto *c         = Vultr::Platform::init_upload_context(Vultr::engine()->context);
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Material *>();
	while (true)
	{
		auto [resource, path, is_kill_request] = allocator->wait_pop_load_queue();

		if (is_kill_request)
		{
			Vultr::Platform::destroy_upload_context(c);
			return;
		}

		Vultr::String material_src;
		{
			auto res = Vultr::try_fread_all(Vultr::get_material_resource(project, path), &material_src);
			if (res.is_error())
			{
				allocator->add_loaded_resource_error(resource, res.get_error());
				continue;
			}
		}

		auto shader_path = Vultr::split(material_src, "\n")[0];
		auto shader      = Vultr::Resource<Vultr::Platform::Shader *>(Vultr::Path(shader_path));
		shader.wait_loaded();

		if check (Vultr::Platform::try_load_material(c, shader, material_src), auto *mat, auto err)
		{
			if (!allocator->add_loaded_resource(resource, mat).has_value())
			{
				Vultr::Platform::destroy_material(Vultr::engine()->context, mat);
			}
		}
		else
		{
			allocator->add_loaded_resource_error(resource, err);
			continue;
		}
	}
}

static void material_free_thread()
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Material *>();
	while (true)
	{
		auto *mat = allocator->wait_pop_free_queue();

		if (mat == (void *)-1)
		{
			return;
		}

		Vultr::Platform::destroy_material(Vultr::engine()->context, mat);
	}
}

static void shader_loader_thread(const Vultr::Project *project)
{
	auto *c         = Vultr::Platform::init_upload_context(Vultr::engine()->context);
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Shader *>();
	while (true)
	{

		auto [resource, path, is_kill_request] = allocator->wait_pop_load_queue();
		if (is_kill_request)
		{
			Vultr::Platform::destroy_upload_context(c);
			return;
		}

		Vultr::Platform::CompiledShaderSrc shader_src{};
		auto [vert, frag] = Vultr::get_shader_resource(project, path);
		{
			auto res = Vultr::try_fread_all(vert, &shader_src.vert_src);
			if (res.is_error())
			{
				allocator->add_loaded_resource_error(resource, res.get_error());
				continue;
			}
		}
		{
			auto res = Vultr::try_fread_all(frag, &shader_src.frag_src);
			if (res.is_error())
			{
				allocator->add_loaded_resource_error(resource, res.get_error());
				continue;
			}
		}

		if check (Vultr::Platform::try_reflect_shader(shader_src), auto reflection, auto err)
		{
			if check (Vultr::Platform::try_load_shader(Vultr::engine()->context, shader_src, reflection), auto *shader, auto err)
			{
				if (allocator->add_loaded_resource(resource, shader).is_error())
				{
					Vultr::Platform::destroy_shader(Vultr::engine()->context, shader);
				}
			}
			else
			{
				allocator->add_loaded_resource_error(resource, err);
				continue;
			}
		}
		else
		{
			allocator->add_loaded_resource_error(resource, err);
			continue;
		}
	}
}

static void shader_free_thread()
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Shader *>();
	while (true)
	{
		auto *shader = allocator->wait_pop_free_queue();

		if (shader == (void *)-1)
		{
			return;
		}

		Vultr::Platform::destroy_shader(Vultr::engine()->context, shader);
	}
}

static void texture_loader_thread(const Vultr::Project *project)
{
	auto *c         = Vultr::Platform::init_upload_context(Vultr::engine()->context);
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Texture *>();
	while (true)
	{
		auto [resource, path, is_kill_request] = allocator->wait_pop_load_queue();

		if (is_kill_request)
		{
			Vultr::Platform::destroy_upload_context(c);
			return;
		}

		Vultr::Buffer src;
		{
			auto res = Vultr::try_fread_all(Vultr::get_texture_resource(project, path), &src);
			if (res.is_error())
			{
				allocator->add_loaded_resource_error(resource, res.get_error());
				continue;
			}
		}
		auto *texture = Vultr::Platform::init_texture(c, *(f64 *)&src[0], *(f64 *)&src[sizeof(f64)], Vultr::Platform::TextureFormat::RGBA8);
		Vultr::Platform::fill_texture(c, texture, &src[sizeof(f64) * 2], src.size() - 2 * sizeof(f64));

		if (allocator->add_loaded_resource(resource, texture).is_error())
		{
			Vultr::Platform::destroy_texture(Vultr::engine()->context, texture);
		}
	}
}

static void texture_free_thread()
{
	auto *allocator = Vultr::resource_allocator<Vultr::Platform::Texture *>();
	while (true)
	{
		auto *texture = allocator->wait_pop_free_queue();

		if (texture == (void *)-1)
		{
			return;
		}

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

		Vultr::open_windowed("Vultr Game Engine");
		if check (Vultr::load_game(build_dir, resource_dir), auto project, auto err)
		{

			Vultr::init_resource_allocators();

			Platform::Thread mesh_loading_thread(mesh_loader_thread, &project);
			mesh_loading_thread.detach();
			Platform::Thread mesh_freeing_thread(mesh_free_thread);
			mesh_freeing_thread.detach();
			Platform::Thread material_loading_thread(material_loader_thread, &project);
			material_loading_thread.detach();
			Platform::Thread material_freeing_thread(material_free_thread);
			material_freeing_thread.detach();
			Platform::Thread shader_loading_thread(shader_loader_thread, &project);
			shader_loading_thread.detach();
			Platform::Thread shader_freeing_thread(shader_free_thread);
			shader_freeing_thread.detach();
			Platform::Thread texture_loading_thread(texture_loader_thread, &project);
			texture_loading_thread.detach();
			Platform::Thread texture_freeing_thread(texture_free_thread);
			texture_freeing_thread.detach();

			EditorRuntime runtime{};
			runtime.render_system  = RenderSystem::init();
			runtime.upload_context = Vultr::Platform::init_upload_context(Vultr::engine()->context);
			runtime.imgui_c        = Platform::init_imgui(engine()->window, runtime.upload_context, EditorResources::GET_ROBOTO_TTF(), EditorResources::ROBOTO_TTF_LEN, 15);

			EditorWindowState state;
			begin_resource_import(&project, &state);

			project.register_components();

			init_windows(&runtime, &project, &state);

			while (!Platform::window_should_close(engine()->window))
			{
				Platform::poll_events(engine()->window);
				Input::update_input(Input::input_manager(), state.render_window_offset, state.render_window_size);
				auto dt = Platform::update_window(engine()->window);

				if (state.hot_reload_fence.try_acquire())
				{
					if (state.playing)
						project.update(state.game_memory, dt);

					update_windows(&state, dt);
					state.hot_reload_fence.release();
				}

				if check (Platform::begin_cmd_buffer(engine()->window), auto *cmd, auto _)
				{
					if (state.hot_reload_fence.try_acquire())
					{
						if (state.playing)
						{
							RenderSystem::update(cmd, runtime.render_system);
						}
						else
						{
							RenderSystem::update(state.editor_camera, state.editor_camera_transform, cmd, runtime.render_system);
						}
						state.hot_reload_fence.release();
					}

					Platform::begin_window_framebuffer(cmd);
					render_windows(cmd, runtime.render_system, &project, &state, &runtime, dt);
					Platform::end_framebuffer(cmd);

					Platform::end_cmd_buffer(cmd);
				}
				else
				{
					RenderSystem::reinitialize(runtime.render_system);
				}
			}

			if (state.hot_reload_fence.try_acquire())
			{
				if (state.started)
					project.destroy(state.game_memory);

				world()->component_manager.deregister_non_system_components();
				state.hot_reload_fence.release();
			}

			Platform::wait_idle(engine()->context);
			resource_allocator<Platform::Mesh *>()->kill_loading_threads();
			resource_allocator<Platform::Mesh *>()->kill_freeing_threads();
			resource_allocator<Platform::Material *>()->kill_loading_threads();
			resource_allocator<Platform::Material *>()->kill_freeing_threads();
			resource_allocator<Platform::Shader *>()->kill_loading_threads();
			resource_allocator<Platform::Shader *>()->kill_freeing_threads();
			resource_allocator<Platform::Texture *>()->kill_loading_threads();
			resource_allocator<Platform::Texture *>()->kill_freeing_threads();

			destroy_windows(&state);
			RenderSystem::destroy(runtime.render_system);
			Platform::destroy_imgui(engine()->context, runtime.imgui_c);
			Platform::destroy_upload_context(runtime.upload_context);
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
