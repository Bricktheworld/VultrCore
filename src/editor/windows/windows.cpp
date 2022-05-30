#include "windows.h"
#include <editor/res/resources.h>
#include <ecs/serialization.h>
#include <glm/gtc/type_ptr.hpp>
#include "scene_window.cpp"
#include "entity_hierarchy.cpp"
#include "component_inspector.cpp"
#include "resource_browser.cpp"

namespace ImGui
{
	bool InputText(const Vultr::StringView &label, Vultr::String *string)
	{
		size_t buf_size = string->size() + 256;
		char buf[buf_size];
		strncpy(buf, string->c_str(), string->size());
		if (ImGui::InputText(label.c_str(), buf, buf_size))
		{
			*string = buf;
			return true;
		}
		return false;
	}
} // namespace ImGui

namespace Vultr
{
	ProgressState *begin_progress_bar(EditorWindowState *state, const StringView &title)
	{
		ASSERT(!state->progress_state, "Currently we don't support multiple progress bars in the editor!");
		state->progress_state               = ProgressState();
		state->progress_state.value().title = title;
		return &state->progress_state.value();
	}

	void end_progress_bar(EditorWindowState *state, ProgressState *progress)
	{
		ASSERT(state->progress_state, "Cannot end non-existent progress bar!");
		state->progress_state.value().done = true;
	}

	void display_error(EditorWindowState *state, const StringView &title, const String &message) { state->error_message = ErrorMessage{.title = title, .message = message}; }

	bool serialize_current_scene(EditorWindowState *state)
	{
		if (!state->started)
		{
			if let (const auto &scene_path, state->scene_path)
			{
				auto out = FileOutputStream(state->scene_path.value(), StreamFormat::BINARY, StreamWriteMode::OVERWRITE);
				auto res = serialize_world_yaml(world(), &out);
				if (res.is_error())
				{
					display_error(state, "Scene Serialization Failed", String(res.get_error().message));
					return false;
				}
				return true;
			}
			else
			{
				display_error(state, "No Scene Open", String("Please open a scene before saving."));
				return false;
			}
		}
		else
		{
			display_error(state, "Cannot Save When Playing", String("In order to save the current scene, please exit play mode."));
			return false;
		}
	}

	bool load_scene(EditorWindowState *state, const Path &file)
	{
		state->scene_path = file;
		String src;
		fread_all(file, &src);
		world()->component_manager.destroy_component_arrays();
		world()->entity_manager = EntityManager();
		auto res                = read_world_yaml(src, world());
		if (res.is_error())
		{
			display_error(state, "Failed To Load Scene", String(res.get_error().message));
			return false;
		}
		else
		{
			return true;
		}
	}

	static void on_key_press(void *data, Input::Key key, Input::Action action, Input::Key modifiers)
	{
		auto *state = static_cast<EditorWindowState *>(data);
		if (action == Input::ACTION_PRESS)
		{
			if ((modifiers & Input::KEY_CONTROL) == Input::KEY_CONTROL)
			{
				if (key == Input::KEY_S)
				{
					serialize_current_scene(state);
				}
				else if (key == Input::KEY_D)
				{
					if let (auto entity, state->selected_entity)
					{
						Entity duplicate;
						if (has_parent(entity))
						{
							duplicate = create_parented_entity(*get_label(entity), get_parent(entity).value(), Transform{});
						}
						else
						{
							duplicate = create_entity(*get_label(entity));
						}
						auto signature = get_signature(entity);
						auto *em       = &world()->entity_manager;
						auto *cm       = &world()->component_manager;
						em->add_signature(duplicate, signature);

						for (auto [type_id, index] : cm->type_to_index)
						{
							if (!signature.at(index))
								continue;

							auto *array = cm->component_arrays[index];
							auto type   = array->rtti_type;
							void *dest;
							if (!array->m_entity_to_index.contains(duplicate))
							{
								auto component_index = array->m_size;
								array->m_entity_to_index.set(duplicate, component_index);
								array->m_index_to_entity.set(component_index, duplicate);
								dest = static_cast<byte *>(array->m_array) + (component_index * type.size());
								array->m_size++;
							}
							else
							{
								auto component_index = array->m_entity_to_index.get(duplicate);
								dest                 = static_cast<byte *>(array->m_array) + (component_index * type.size());
							}
							void *src = static_cast<byte *>(array->m_array) + (type.size() * array->m_entity_to_index.get(entity));
							type.copy_constructor(dest, src);
						}
					}
					else
					{
					}
				}
			}
		}
	}

	void init_windows(EditorRuntime *runtime, Project *project, EditorWindowState *state)
	{
		state->key_listener = Platform::register_key_callback(engine()->window, state, on_key_press);
		resource_window_init(project, state);
		state->texture = Platform::init_texture(runtime->upload_context, EditorResources::TEXTURE_PNG_WIDTH, EditorResources::TEXTURE_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->texture, EditorResources::GET_TEXTURE_PNG(), EditorResources::TEXTURE_PNG_LEN);
		state->cpp_source = Platform::init_texture(runtime->upload_context, EditorResources::CPP_PNG_WIDTH, EditorResources::CPP_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->cpp_source, EditorResources::GET_CPP_PNG(), EditorResources::CPP_PNG_LEN);
		state->shader = Platform::init_texture(runtime->upload_context, EditorResources::SHADER_PNG_WIDTH, EditorResources::SHADER_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->shader, EditorResources::GET_SHADER_PNG(), EditorResources::SHADER_PNG_LEN);
		state->file = Platform::init_texture(runtime->upload_context, EditorResources::FILE_PNG_WIDTH, EditorResources::FILE_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->file, EditorResources::GET_FILE_PNG(), EditorResources::FILE_PNG_LEN);
		state->folder = Platform::init_texture(runtime->upload_context, EditorResources::FOLDER_PNG_WIDTH, EditorResources::FOLDER_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->folder, EditorResources::GET_FOLDER_PNG(), EditorResources::FOLDER_PNG_LEN);
		state->mesh = Platform::init_texture(runtime->upload_context, EditorResources::MESH_PNG_WIDTH, EditorResources::MESH_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(runtime->upload_context, state->mesh, EditorResources::GET_MESH_PNG(), EditorResources::MESH_PNG_LEN);
	}

	void update_windows(EditorWindowState *state, f64 dt) { scene_window_update(state, dt); }

	void render_windows(Platform::CmdBuffer *cmd, RenderSystem::Component *render_system, Project *project, EditorWindowState *state, EditorRuntime *runtime, f64 dt)
	{
		Platform::imgui_begin_frame(cmd, runtime->imgui_c);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
										ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("VultrDockspace", &state->dockspace_open, window_flags);
		ImGui::PopStyleVar(3);

		ImGui::Text("%.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save Scene"))
				{
					serialize_current_scene(state);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Run"))
			{
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (state->progress_state.has_value())
		{
			auto *progress_state = &state->progress_state.value();
			if (ImGui::BeginPopupModal(progress_state->title))
			{
				ImGui::Text("%s", progress_state->message.c_str());
				ImGui::ProgressBar(static_cast<f32>(progress_state->progress) / static_cast<f32>(progress_state->total));
				if (progress_state->done)
				{
					ImGui::CloseCurrentPopup();
					state->progress_state = None;
				}
				ImGui::EndPopup();
			}
			else
			{
				ImGui::OpenPopup(progress_state->title);
			}
		}

		if (state->error_message.has_value())
		{
			auto *error_message = &state->error_message.value();
			if (ImGui::BeginPopupModal(error_message->title))
			{
				ImGui::Text("%s", error_message->message.c_str());
				if (ImGui::Button("Ok"))
				{
					state->error_message = None;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			else
			{
				ImGui::OpenPopup(error_message->title);
			}
		}
		else
		{
		}

		//		if (ImGui::BeginPopupModal("Hot Reloading Game..."))
		//		{
		//			ImGui::Text("Hot reloading game, please wait.");
		//			if (!state->hot_reloading)
		//				ImGui::CloseCurrentPopup();
		//			ImGui::EndPopup();
		//		}
		//		else
		//		{
		//			if (state->hot_reloading)
		//			{
		//				ImGui::OpenPopup("Hot Reloading Game...");
		//			}
		//		}

		auto dockspace = ImGui::GetID("HUB_DockSpace");
		ImGui::DockSpace(dockspace, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoResize);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		scene_window_draw(render_system, project, state, runtime);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow();

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		entity_hierarchy_window_draw(project, state);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		component_inspector_window_draw(project, state);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		resource_browser_window_draw(project, state);

		ImGui::End();

		Platform::imgui_end_frame(cmd, runtime->imgui_c);
	}

	void destroy_windows(EditorWindowState *state)
	{
		auto *c = engine()->context;

		Platform::destroy_texture(c, state->texture);
		Platform::destroy_texture(c, state->cpp_source);
		Platform::destroy_texture(c, state->shader);
		Platform::destroy_texture(c, state->file);
		Platform::destroy_texture(c, state->folder);
		Platform::destroy_texture(c, state->mesh);
	}

	static u32 count_resource_imports(const Project *project, const Path &dir)
	{
		u32 count = 0;
		for (auto entry : DirectoryIterator(dir))
		{
			if (entry.is_file())
			{
				if (needs_reimport(project, entry))
					count++;
			}
			else
			{
				count += count_resource_imports(project, entry);
			}
		}
		return count;
	}

	static void map_asset_uuids(Project *project, const Path &dir)
	{
		for (auto entry : DirectoryIterator(dir))
		{
			if (entry.is_file())
			{
				if check (get_resource_metadata(entry), auto metadata, auto _)
				{
					project->asset_map.set(metadata.uuid, entry);
				}
				else
				{
				}
			}
			else
			{
				map_asset_uuids(project, entry);
			}
		}
	}

	static void resource_import_thread(Project *project, EditorWindowState *state, ProgressState *progress_state)
	{
		auto res = makedir(project->build_resource_dir);
		progress_state->message = "Importing assets, please wait...";
		if (!res.is_error())
		{
			progress_state->total = count_resource_imports(project, project->resource_dir);
			auto res              = import_dir(project, project->resource_dir, &progress_state->progress);
			if (res.is_error())
				display_error(state, "Failed to import resources!", String(res.get_error().message));
		}
		else
		{
			display_error(state, "Failed to import resources!", String(res.get_error().message));
		}

		map_asset_uuids(project, project->resource_dir);

		end_progress_bar(state, progress_state);
		state->resource_browser_state.need_refresh = true;
	}

	void begin_resource_import(Project *project, EditorWindowState *state)
	{
		auto *progress_state = begin_progress_bar(state, "Importing Resources");
		Platform::Thread thread(resource_import_thread, project, state, progress_state);
		thread.detach();
	}
} // namespace Vultr
