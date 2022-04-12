#include "../rendering.h"
#include "stb_image/stb_image.h"

namespace Vultr
{
	namespace Platform
	{
		ErrorOr<Texture *> load_texture_file(UploadContext *c, const Path &path, TextureFormat format)
		{
			s32 width, height, channels;
			stbi_set_flip_vertically_on_load(true);
			byte *buffer = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

			if (buffer == nullptr)
				return Error("stbi failed to load texture!");

			auto *texture = init_texture(c, width, height, format);
			fill_texture(c, texture, buffer);
			stbi_image_free(buffer);

			return texture;
		}

		ErrorOr<Texture *> load_texture_memory(UploadContext *c, byte *data, u64 size, TextureFormat format)
		{
			s32 width  = 0;
			s32 height = 0;
			stbi_set_flip_vertically_on_load(true);
			byte *buffer = stbi_load_from_memory(data, static_cast<s32>(size), &width, &height, nullptr, 4);

			if (buffer == nullptr)
				return Error("stbi failed to load texture!");

			auto *texture = init_texture(c, width, height, format);
			fill_texture(c, texture, data);
			stbi_image_free(buffer);

			return texture;
		}

	} // namespace Platform

} // namespace Vultr
