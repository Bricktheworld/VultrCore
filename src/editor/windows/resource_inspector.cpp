#include "../editor.h"

namespace Vultr
{
	static bool draw_texture_format_option(Platform::TextureFormat format, Platform::TextureFormat *out)
	{
		bool ret           = false;
		bool is_selected   = format == *out;
		const char *string = Platform::texture_format_to_string(format);
		if (ImGui::Selectable(string, is_selected))
		{
			ret  = true;
			*out = format;
		}

		if (is_selected)
			ImGui::SetItemDefaultFocus();
		return ret;
	}

	void resource_inspector_window_draw(Editor *e, Platform::CmdBuffer *cmd)
	{
		ImGui::Begin("Asset Inspector");

		auto *state = &e->resource_browser;

		if let (auto index, state->selected_index)
		{
			if (index >= state->dirs.size())
			{
				u32 correct_index = index - state->dirs.size();
				auto *file        = &state->files[correct_index];
				ImGui::Text("%s", file->path.c_str());

				Platform::UUID_String uuid_str;
				Platform::stringify_uuid(file->metadata.uuid, uuid_str);

				ImGui::Text("UUID: %s", uuid_str);

				auto resource_type = file->metadata.resource_type;
				ImGui::Text("%s", resource_type_to_string(resource_type));

				if (resource_type == ResourceType::TEXTURE)
				{
					auto texture_metadata = file->texture_metadata;

					if (ImGui::BeginCombo("Format", Platform::texture_format_to_string(texture_metadata.texture_format)))
					{

#define TEXTURE_FORMAT_OPTION(format) file->updated_properties = draw_texture_format_option(Platform::TextureFormat::format, &file->texture_metadata.texture_format) || file->updated_properties

						TEXTURE_FORMAT_OPTION(RGB8);
						TEXTURE_FORMAT_OPTION(RGB16);
						TEXTURE_FORMAT_OPTION(RGBA8);
						TEXTURE_FORMAT_OPTION(RGBA16);
						TEXTURE_FORMAT_OPTION(SRGB8);
						TEXTURE_FORMAT_OPTION(SRGBA8);
						TEXTURE_FORMAT_OPTION(RGB8_CUBEMAP);
						TEXTURE_FORMAT_OPTION(RGBA8_CUBEMAP);
						TEXTURE_FORMAT_OPTION(RGB16_CUBEMAP);
						TEXTURE_FORMAT_OPTION(RGBA16_CUBEMAP);
						TEXTURE_FORMAT_OPTION(SRGB8_CUBEMAP);
						TEXTURE_FORMAT_OPTION(SRGBA8_CUBEMAP);
						// We are intentionally omitting depth format here because that's not really a
						// valid format for a texture you would be supplying as an asset.

						ImGui::EndCombo();
					}
				}

				if (file->updated_properties)
				{
					if (ImGui::Button("Save"))
					{
						auto res = save_texture_metadata(file->path, file->metadata, file->texture_metadata);
						if (res)
						{
							file->updated_properties = false;
							begin_resource_import(e);
						}
						else
						{
							fprintf(stderr, "Failed to save texture metadata: %s\n", res.get_error().message.c_str());
						}
					}
				}
			}
		}
		else
		{
			ImGui::Text("No selected asset");
		}

		ImGui::End();
	}
} // namespace Vultr
