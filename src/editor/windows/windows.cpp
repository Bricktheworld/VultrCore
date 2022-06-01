#include "../editor.h"
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
	ProgressState *begin_progress_bar(Editor *e, const StringView &title)
	{
		ASSERT(!e->progress_state, "Currently we don't support multiple progress bars in the editor!");
		e->progress_state               = ProgressState();
		e->progress_state.value().title = title;
		return &e->progress_state.value();
	}

	void end_progress_bar(Editor *e, ProgressState *progress)
	{
		ASSERT(e->progress_state, "Cannot end non-existent progress bar!");
		e->progress_state.value().done = true;
	}

	void display_error(Editor *e, const StringView &title, const String &message) { e->error_message = ErrorMessage{.title = title, .message = message}; }

	bool serialize_current_scene(Editor *e)
	{
		if (!e->started)
		{
			if let (const auto &scene_path, e->scene_path)
			{
				auto out = FileOutputStream(e->scene_path.value(), StreamFormat::BINARY, StreamWriteMode::OVERWRITE);
				auto res = serialize_world_yaml(world(), &out);
				if (res.is_error())
				{
					display_error(e, "Scene Serialization Failed", String(res.get_error().message));
					return false;
				}
				return true;
			}
			else
			{
				display_error(e, "No Scene Open", String("Please open a scene before saving."));
				return false;
			}
		}
		else
		{
			display_error(e, "Cannot Save When Playing", String("In order to save the current scene, please exit play mode."));
			return false;
		}
	}

	bool load_scene(Editor *e, const Path &file)
	{
		e->scene_path = file;
		String src;
		fread_all(file, &src);
		world()->component_manager.destroy_component_arrays();
		world()->entity_manager = EntityManager();
		auto res                = read_world_yaml(src, world());
		if (res.is_error())
		{
			display_error(e, "Failed To Load Scene", String(res.get_error().message));
			return false;
		}
		else
		{
			return true;
		}
	}

	static void on_key_press(void *data, Input::Key key, Input::Action action, Input::Key modifiers)
	{
		auto *e = static_cast<Editor *>(data);
		if (action == Input::ACTION_PRESS)
		{
			if ((modifiers & Input::KEY_CONTROL) == Input::KEY_CONTROL)
			{
				if (key == Input::KEY_S)
				{
					serialize_current_scene(e);
				}
				else if (key == Input::KEY_D)
				{
					if let (auto entity, e->selected_entity)
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

	static void load_resources(Editor *e)
	{
		auto *rm    = &e->resource_manager;
		rm->texture = Platform::init_texture(rm->upload_context, EditorResources::TEXTURE_PNG_WIDTH, EditorResources::TEXTURE_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(rm->upload_context, rm->texture, EditorResources::GET_TEXTURE_PNG(), EditorResources::TEXTURE_PNG_LEN);
		rm->cpp_source = Platform::init_texture(rm->upload_context, EditorResources::CPP_PNG_WIDTH, EditorResources::CPP_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(rm->upload_context, rm->cpp_source, EditorResources::GET_CPP_PNG(), EditorResources::CPP_PNG_LEN);
		rm->shader = Platform::init_texture(rm->upload_context, EditorResources::SHADER_PNG_WIDTH, EditorResources::SHADER_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(rm->upload_context, rm->shader, EditorResources::GET_SHADER_PNG(), EditorResources::SHADER_PNG_LEN);
		rm->file = Platform::init_texture(rm->upload_context, EditorResources::FILE_PNG_WIDTH, EditorResources::FILE_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(rm->upload_context, rm->file, EditorResources::GET_FILE_PNG(), EditorResources::FILE_PNG_LEN);
		rm->folder = Platform::init_texture(rm->upload_context, EditorResources::FOLDER_PNG_WIDTH, EditorResources::FOLDER_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(rm->upload_context, rm->folder, EditorResources::GET_FOLDER_PNG(), EditorResources::FOLDER_PNG_LEN);
		rm->mesh = Platform::init_texture(rm->upload_context, EditorResources::MESH_PNG_WIDTH, EditorResources::MESH_PNG_HEIGHT, Platform::TextureFormat::RGBA8);
		Platform::fill_texture(rm->upload_context, rm->mesh, EditorResources::GET_MESH_PNG(), EditorResources::MESH_PNG_LEN);
	}

	void init_windows(Editor *e)
	{
		e->key_listener = Input::register_key_callback(e, on_key_press);
		resource_window_init(e);
		load_resources(e);
	}

	void update_windows(Editor *e, f64 dt) { scene_window_update(e, dt); }

	void render_windows(Editor *e, Platform::CmdBuffer *cmd, RenderSystem::Component *render_system, f64 dt)
	{
		Platform::imgui_begin_frame(cmd, e->imgui_c);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
										ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("VultrDockspace", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		ImGui::Text("%.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save Scene"))
				{
					serialize_current_scene(e);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Run"))
			{
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (e->progress_state.has_value())
		{
			auto *progress_state = &e->progress_state.value();
			if (ImGui::BeginPopupModal(progress_state->title))
			{
				ImGui::Text("%s", progress_state->message.c_str());
				ImGui::ProgressBar(static_cast<f32>(progress_state->progress) / static_cast<f32>(progress_state->total));
				if (progress_state->done)
				{
					ImGui::CloseCurrentPopup();
					e->progress_state = None;
				}
				ImGui::EndPopup();
			}
			else
			{
				ImGui::OpenPopup(progress_state->title);
			}
		}
		else if (e->error_message.has_value())
		{
			auto *error_message = &e->error_message.value();
			if (ImGui::BeginPopupModal(error_message->title))
			{
				ImGui::Text("%s", error_message->message.c_str());
				if (ImGui::Button("Ok"))
				{
					e->error_message = None;
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
		scene_window_draw(e, render_system);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow();

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		entity_hierarchy_window_draw(e);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		component_inspector_window_draw(e);

		ImGui::SetNextWindowDockID(dockspace, ImGuiCond_FirstUseEver);
		resource_browser_window_draw(e, cmd);

		ImGui::End();

		Platform::begin_window_framebuffer(cmd);
		Platform::imgui_end_frame(cmd, e->imgui_c);
		Platform::end_framebuffer(cmd);
	}

	void destroy_windows(Editor *e)
	{
		resource_browser_destroy(e);
		auto *c  = engine()->context;

		auto *rm = &e->resource_manager;
		Platform::destroy_texture(c, rm->texture);
		Platform::destroy_texture(c, rm->cpp_source);
		Platform::destroy_texture(c, rm->shader);
		Platform::destroy_texture(c, rm->file);
		Platform::destroy_texture(c, rm->folder);
		Platform::destroy_texture(c, rm->mesh);
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

	static void resource_import_thread(Editor *e, ProgressState *progress_state)
	{
		auto res                = makedir(e->project.build_resource_dir);
		progress_state->message = "Importing assets, please wait...";
		if (!res.is_error())
		{
			progress_state->total = count_resource_imports(&e->project, e->project.resource_dir);
			auto res              = import_dir(&e->project, e->project.resource_dir, &progress_state->progress);
			if (res.is_error())
				display_error(e, "Failed to import resources!", String(res.get_error().message));
		}
		else
		{
			display_error(e, "Failed to import resources!", String(res.get_error().message));
		}

		map_asset_uuids(&e->project, e->project.resource_dir);

		end_progress_bar(e, progress_state);
		e->resource_browser.need_refresh = true;
	}

	void begin_resource_import(Editor *e)
	{
		auto *progress_state = begin_progress_bar(e, "Importing Resources");
		Platform::Thread thread(resource_import_thread, e, progress_state);
		thread.detach();
	}
} // namespace Vultr
