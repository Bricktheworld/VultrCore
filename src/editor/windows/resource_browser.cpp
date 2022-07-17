#include "../editor.h"

namespace Vultr
{

	static void change_dir(Editor *e, Path dir)
	{
		auto *state           = &e->resource_browser;
		state->selected_index = None;
		state->current_dir    = dir;
		state->dirs.clear();
		state->files.clear();
		state->need_refresh = false;
		for (auto path : DirectoryIterator(dir))
		{
			if (path.is_directory())
			{
				state->dirs.push_back(path);
			}
			else
			{
				if (is_asset_imported(&e->project, path))
				{
					auto metadata     = get_resource_metadata(path).value();
					ResourceFile file = {
						.path                 = path,
						.uuid                 = metadata.uuid,
						.metadata             = metadata,
						.rendered_framebuffer = None,
					};

					if (metadata.resource_type == ResourceType::TEXTURE)
					{
						file.texture_metadata = get_texture_metadata(path).value_or(TextureMetadata{});
					}

					state->files.push_back(file);
				}
			}
		}
	}

	void resource_browser_window_init(Editor *e)
	{
		change_dir(e, e->project.resource_dir);
		auto *state            = &e->resource_browser;
		state->material_sphere = Platform::init_sphere(engine()->upload_context, 1, 50, 50);
	}

	static void asset_drag_drop_src(const MetadataHeader &metadata)
	{
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(resource_type_to_string(metadata.resource_type), metadata.uuid.m_uuid, sizeof(metadata.uuid.m_uuid));
			ImGui::EndDragDropSource();
		}
	}

	static Platform::Texture *get_file_resource_texture(Editor *e, Platform::CmdBuffer *cmd, ResourceFile *file, u32 preview_size)
	{
		auto *rm = &e->resource_manager;
		switch (file->metadata.resource_type)
		{
			case ResourceType::TEXTURE:
			{
				if (file->resource_texture.empty())
				{
					file->resource_texture = Resource<Platform::Texture *>(file->uuid);
				}

				auto *texture = file->resource_texture.value_or(rm->texture);

				if (Platform::is_cubemap(Platform::get_texture_format(texture)))
					return rm->texture;

				return texture;
			}
			case ResourceType::SHADER:
			case ResourceType::COMPUTE_SHADER:
				return rm->shader;
			case ResourceType::MATERIAL:
			{
				//				if (!file->rendered_framebuffer)
				//				{
				//					Vector<Platform::AttachmentDescription> attachments({{.format = Platform::TextureFormat::SRGBA8}});
				//					file->rendered_framebuffer = Platform::init_framebuffer(engine()->context, attachments, preview_size, preview_size);
				//				}
				//
				//				if (file->resource_material.empty())
				//					file->resource_material = Resource<Platform::Material *>(file->uuid);
				//
				//				if (file->resource_material.loaded())
				//				{
				//					auto camera_transform = Transform{.position = Vec3(0, 0, -1)};
				//					auto camera_component = Camera{};
				//					auto light_transform  = Transform{};
				//					auto light_component  = DirectionalLight{
				//						 .intensity = 3,
				//						 .specular  = 1,
				//						 .ambient   = 0.140,
				//                    };
				//
				//					Platform::CameraUBO camera_ubo{
				//						.position  = Vec4(camera_transform.position, 0),
				//						.view      = view_matrix(camera_transform),
				//						.proj      = projection_matrix(camera_component, preview_size, preview_size),
				//						.view_proj = camera_ubo.proj * camera_ubo.view,
				//
				//					};
				//					Platform::DirectionalLightUBO light_ubo{
				//						.direction = Vec4(forward(light_transform), 0),
				//						.diffuse   = light_component.diffuse,
				//						.specular  = light_component.specular,
				//						.intensity = light_component.intensity,
				//						.ambient   = light_component.ambient,
				//					};
				//					//					Platform::update_default_descriptor_set(cmd, &camera_ubo, &light_ubo);
				//
				//					//					Platform::begin_framebuffer(cmd, file->rendered_framebuffer.value(), Vec4(1));
				//					//					Platform::PushConstant push{
				//					//						.model = model_matrix({}),
				//					//					};
				//					//					Platform::bind_material(cmd, file->resource_material.value(), {}, push);
				//					//					Platform::draw_mesh(cmd, e->resource_browser.material_sphere);
				//					//					Platform::end_framebuffer(cmd);
				//
				//					//					return get_attachment_texture(file->rendered_framebuffer.value(), 0);
				//					return rm->file;
				//				}
				//				else
				//				{
				//					// TODO(Brandon): Get an actual icon for materials.
				//					return rm->file;
				//				}
				return rm->file;
			}
			case ResourceType::MESH:
			{
				if (!file->rendered_framebuffer)
				{
					Vector<Platform::AttachmentDescription> attachments({{.format = Platform::TextureFormat::SRGBA8}});
					file->rendered_framebuffer = Platform::init_framebuffer(engine()->context, attachments, preview_size, preview_size);
				}

				if (file->resource_mesh.empty())
					file->resource_mesh = Resource<Platform::Mesh *>(file->uuid);

				if (file->resource_mesh.loaded())
				{
					Platform::begin_framebuffer(cmd, file->rendered_framebuffer.value(), Vec4(1));
					Platform::end_framebuffer(cmd);
					return get_attachment_texture(file->rendered_framebuffer.value(), 0);
				}
				else
				{
					return rm->mesh;
				}
			}
			case ResourceType::CPP_SRC:
				return rm->cpp_source;
			case ResourceType::SCENE:
				// TODO(Brandon): Get an actual icon for scenes.
				return rm->file;
			case ResourceType::OTHER:
			default:
				return rm->file;
		}
	}

	static void resource_browser_refresh(Editor *e)
	{
		if (!e->resource_browser.need_refresh)
			return;
		change_dir(e, e->resource_browser.current_dir);
	}

	void resource_browser_window_draw(Editor *e, Platform::CmdBuffer *cmd)
	{
		resource_browser_refresh(e);
		ImGui::Begin("Asset Browser");
		auto *state = &e->resource_browser;

		if (ImGui::Button("<-"))
		{
			if check (get_parent(state->current_dir), auto parent, auto err)
			{
				change_dir(e, parent);
				ImGui::End();
				return;
			}
			else
			{
			}
		}
		ImGui::SameLine();

		if (ImGui::Button("Reimport"))
			begin_resource_import(e);

		static constexpr f32 PADDING        = 16.0f;
		static constexpr f32 THUMBNAIL_SIZE = 128.0f;
		static constexpr f32 CELL_SIZE      = THUMBNAIL_SIZE + PADDING;

		f32 panel_width                     = ImGui::GetContentRegionAvail().x;
		u32 column_count                    = max<u32>(static_cast<u32>(panel_width / CELL_SIZE), 1);

		ImGui::Columns(column_count, 0, false);

		for (u32 i = 0; i < state->dirs.size(); i++)
		{
			ImGui::PushID(i);

			auto *texture = Platform::imgui_get_texture_id(e->resource_manager.folder);

			auto pos      = ImGui::GetCursorPos();
			ImGui::Image(texture, {THUMBNAIL_SIZE, THUMBNAIL_SIZE}, {0, 1}, {1, 0});

			ImGui::SetCursorPos(pos);
			if (ImGui::Selectable("##", state->selected_index.has_value() && state->selected_index.value() == i, ImGuiSelectableFlags_AllowDoubleClick, {THUMBNAIL_SIZE, THUMBNAIL_SIZE}))
			{
				state->selected_index = i;
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				change_dir(e, state->dirs[i]);
				ImGui::PopID();
				ImGui::End();
				return;
			}

			ImGui::TextWrapped("%s", state->dirs[i].basename().c_str());

			ImGui::NextColumn();

			ImGui::PopID();
		}

		for (u32 i = 0; i < state->files.size(); i++)
		{
			u32 full_index = i + state->dirs.size();
			ImGui::PushID(full_index);

			auto *texture = Platform::imgui_get_texture_id(get_file_resource_texture(e, cmd, &state->files[i], THUMBNAIL_SIZE));

			auto pos      = ImGui::GetCursorPos();
			ImGui::Image(texture, {THUMBNAIL_SIZE, THUMBNAIL_SIZE}, {0, 1}, {1, 0});

			ImGui::SetCursorPos(pos);
			if (ImGui::Selectable("##", state->selected_index.has_value() && state->selected_index.value() == full_index, ImGuiSelectableFlags_AllowDoubleClick, {THUMBNAIL_SIZE, THUMBNAIL_SIZE}))
			{
				state->selected_index = full_index;
			}

			if (ImGui::BeginPopupContextItem())
			{
				if (state->files[i].metadata.resource_type == ResourceType::SHADER)
				{
					if (ImGui::Selectable("New Material"))
					{
						Platform::Shader *shader;
						{
							Buffer src;
							fread_all(get_editor_optimized_path(&e->project, state->files[i].metadata.uuid), &src);
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
						Platform::stringify_uuid(state->files[i].metadata.uuid, shader_uuid);

						String out_buf{};
						out_buf += shader_uuid;

						auto *reflection = Platform::get_reflection_data(shader);

						byte buf[Platform::MAX_MATERIAL_SIZE];
						for (auto &uniform_member : reflection->uniform.members)
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

						begin_resource_import(e);
					}
				}

				ImGui::EndPopup();
			}

			asset_drag_drop_src(state->files[i].metadata);

			ImGui::TextWrapped("%s", state->files[i].path.basename().c_str());

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::End();
	}

	void resource_browser_destroy(Editor *e)
	{
		e->resource_browser.files.clear();
		Platform::destroy_mesh(engine()->context, e->resource_browser.material_sphere);
	}
} // namespace Vultr
