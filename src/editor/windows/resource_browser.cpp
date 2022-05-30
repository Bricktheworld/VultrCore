#include "windows.h"

namespace Vultr
{

#define WIDGET_SIZE 170.0f
#define ASSET_BROWSER_PADDING 2
#define ICON_SIZE 100.0f

	static u32 get_num_cols()
	{

		s32 num_cols = static_cast<s32>(ImGui::GetWindowWidth()) / WIDGET_SIZE - ASSET_BROWSER_PADDING;

		if (num_cols < 1)
			return 1;

		return static_cast<u32>(num_cols);
	}

	static void change_dir(Project *project, Path dir, EditorWindowState *state)
	{
		state->resource_browser_state.selected_index = None;
		state->resource_browser_state.current_dir    = dir;
		state->resource_browser_state.dirs.clear();
		state->resource_browser_state.files.clear();
		state->resource_browser_state.need_refresh = false;
		for (auto path : DirectoryIterator(dir))
		{
			if (path.is_directory())
			{
				state->resource_browser_state.dirs.push_back(path);
			}
			else
			{
				if (is_asset_imported(project, path))
					state->resource_browser_state.files.push_back(path);
			}
		}
	}

	void resource_window_init(Project *project, EditorWindowState *state) { change_dir(project, project->resource_dir, state); }

	static void asset_drag_drop_src(const Path &path)
	{
		if (ImGui::BeginDragDropSource())
		{
			if let (auto metadata, get_resource_metadata(path))
			{
				ImGui::SetDragDropPayload(resource_type_to_string(metadata.resource_type), metadata.uuid.m_uuid, sizeof(metadata.uuid.m_uuid));
			}
			else
			{
				ImGui::SetDragDropPayload("INVALID", UUID().m_uuid, sizeof(UUID::m_uuid));
			}
			ImGui::EndDragDropSource();
		}
	}

	static Platform::Texture *get_texture_for_resource(const Path &path, EditorWindowState *state)
	{
		switch (get_resource_type(path))
		{
			case ResourceType::TEXTURE:
				return state->texture;
			case ResourceType::SHADER:
				return state->shader;
			case ResourceType::MATERIAL:
				// TODO(Brandon): Get an actual icon for materials.
				return state->file;
			case ResourceType::MESH:
				return state->mesh;
			case ResourceType::CPP_SRC:
				return state->cpp_source;
			case ResourceType::SCENE:
				// TODO(Brandon): Get an actual icon for scenes.
				return state->file;
			case ResourceType::OTHER:
				return state->file;
		}
	}

	static void draw_resource(const Path &path, Platform::Texture *texture)
	{
		auto name           = path.basename();

		auto folder_texture = Platform::imgui_get_texture_id(texture);
		ImGui::SameLine((WIDGET_SIZE - ICON_SIZE) / 2);
		ImGui::Image(folder_texture, ImVec2(ICON_SIZE, ICON_SIZE));

		f32 padding_left = (WIDGET_SIZE - ImGui::CalcTextSize(name.c_str()).x) / 2;

		ImGui::SameLine(padding_left);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ICON_SIZE);

		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + WIDGET_SIZE - padding_left);
		ImGui::Text("%s", name.c_str());
		ImGui::PopTextWrapPos();
	}

	static void resource_browser_refresh(Project *project, EditorWindowState *state)
	{
		if (!state->resource_browser_state.need_refresh)
			return;
		change_dir(project, state->resource_browser_state.current_dir, state);
	}

	void resource_browser_window_draw(Project *project, EditorWindowState *editor_state)
	{
		resource_browser_refresh(project, editor_state);
		ImGui::Begin("Asset Browser");
		u32 num_cols = get_num_cols();
		auto *state  = &editor_state->resource_browser_state;

		if (ImGui::Button("Back"))
		{
			if check (get_parent(state->current_dir), auto parent, auto err)
			{
				change_dir(project, parent, editor_state);
				ImGui::End();
				return;
			}
			else
			{
			}
		}
		ImGui::SameLine();

		if (ImGui::Button("Reimport"))
			begin_resource_import(project, editor_state);

		if (ImGui::BeginTable("Asset Table", static_cast<s32>(num_cols)))
		{
			ImGui::TableNextRow();
			u32 col = 0;
			for (u32 i = 0; i < state->files.size(); i++)
			{
				if (col >= num_cols)
				{
					ImGui::TableNextRow();
					col = 0;
				}
				ImGui::TableSetColumnIndex(static_cast<s32>(col));
				auto &file = state->files[i];

				ImGui::PushID(i);
				if (ImGui::Selectable("##", state->selected_index.has_value() && state->selected_index.value() == i, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(WIDGET_SIZE, WIDGET_SIZE)))
				{
					state->selected_index = i;
				}

				if (ImGui::BeginPopupContextItem())
				{
					if (get_resource_type(file) == ResourceType::SHADER)
					{
						if (ImGui::Selectable("New Material"))
						{
							if check (get_resource_metadata(file), auto metadata, auto _)
							{
								Platform::Shader *shader;
								{
									Buffer src;
									fread_all(get_editor_optimized_path(project, metadata.uuid), &src);
									CHECK_UNWRAP(shader, load_editor_optimized_shader(engine()->context, src));
								}

								u32 count    = 1;
								auto new_mat = state->current_dir / "new_material.mat";
								while (exists(new_mat))
								{
									new_mat = state->current_dir / (String("new_material_") + serialize_u64(count) + String(".mat"));
									count++;
								}

								Platform::UUID_String shader_uuid;
								Platform::stringify_uuid(metadata.uuid, shader_uuid);

								String out_buf{};
								out_buf += shader_uuid;

								auto *reflection = Platform::get_reflection_data(shader);

								byte buf[Platform::MAX_MATERIAL_SIZE];
								for (auto &uniform_member : reflection->uniform_members)
								{
									out_buf += "\n" + uniform_member.name + ":" + serialize_member(buf, uniform_member);
								}

								for (auto &sampler_refl : reflection->samplers)
								{
									out_buf += "\n" + sampler_refl.name + ":" + Platform::VULTR_NULL_FILE_HANDLE;
								}

								Platform::destroy_shader(engine()->context, shader);

								fwrite_all(new_mat, out_buf, StreamWriteMode::OVERWRITE);

								ImGui::CloseCurrentPopup();

								begin_resource_import(project, editor_state);
							}
							else
							{
								display_error(editor_state, "Shader has not been import", String("Cannot create material from shader which has not been imported, please import first."));
							}
						}
					}

					ImGui::EndPopup();
				}

				asset_drag_drop_src(file);

				draw_resource(file, get_texture_for_resource(file, editor_state));

				ImGui::PopID();

				col++;
			}

			for (u32 i = 0; i < state->dirs.size(); i++)
			{
				if (col >= num_cols)
				{
					ImGui::TableNextRow();
					col = 0;
				}
				ImGui::TableSetColumnIndex(static_cast<s32>(col));
				u32 full_index = i + state->files.size();
				auto &dir      = state->dirs[i];

				ImGui::PushID(full_index);

				if (ImGui::Selectable("##", state->selected_index.has_value() && state->selected_index.value() == full_index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(WIDGET_SIZE, WIDGET_SIZE)))
				{
					state->selected_index = full_index;
				}

				if (ImGui::BeginDragDropTarget())
				{
					const auto *payload = ImGui::AcceptDragDropPayload("File");

					if (payload != nullptr)
					{
						auto *path = static_cast<char *>(payload->Data);

						change_dir(project, state->current_dir, editor_state);

						ImGui::PopID();
						ImGui::EndTable();
						ImGui::End();
						return;
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					change_dir(project, dir, editor_state);
					ImGui::PopID();
					ImGui::EndTable();
					ImGui::End();
					return;
				}

				draw_resource(dir, editor_state->folder);

				ImGui::PopID();
				col++;
			}

			ImGui::EndTable();
		}

		ImGui::End();
	}
} // namespace Vultr