#include "../rendering.h"
#include "stb_image/stb_image.h"

namespace Vultr
{
	namespace Platform
	{
		ErrorOr<void> import_texture_file(const Path &path, ImportedTexture *out, TextureFormat format, bool flip_on_load)
		{
			s32 width, height, channels;
			stbi_set_flip_vertically_on_load(flip_on_load);
			byte *buffer = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

			if (buffer == nullptr)
				return Error("stbi failed to load texture!");

			out->width      = width;
			out->height     = height;
			size_t img_size = get_pixel_size(format) * width * height;

			out->src.resize(img_size);
			out->src.fill(buffer, img_size);

			stbi_image_free(buffer);
			return Success;
		}

		ErrorOr<void> import_texture_memory(byte *data, u64 size, ImportedTexture *out, TextureFormat format, bool flip_on_load)
		{
			s32 width  = 0;
			s32 height = 0;
			stbi_set_flip_vertically_on_load(flip_on_load);
			byte *buffer = stbi_load_from_memory(data, static_cast<s32>(size), &width, &height, nullptr, 4);

			if (buffer == nullptr)
				return Error("stbi failed to load texture!");

			out->width      = width;
			out->height     = height;
			size_t img_size = get_pixel_size(format) * width * height;

			out->src.resize(img_size);
			out->src.fill(buffer, img_size);

			stbi_image_free(buffer);
			return Success;
		}
	} // namespace Platform
} // namespace Vultr
