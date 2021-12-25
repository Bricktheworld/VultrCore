#pragma once
#include <render/types/texture.h>
#include <string>
#include <types/types.h>
#include <filesystem/file.h>

namespace Vultr
{
	namespace TextureImporter
	{
		bool texture_import(Texture *texture, const TextureSource *source);
		bool texture_import(Texture *texture, const unsigned char *data, u64 size);
		unsigned char *texture_load_file(Texture *texture, const TextureSource *source);
		unsigned char *texture_load_memory(Texture *texture, const unsigned char *data, u64 size);
		void texture_load_gpu(Texture *texture, const unsigned char *data);
		// bool import_skybox(Texture *texture, const std::vector<TextureSource> &paths);
	} // namespace TextureImporter
} // namespace Vultr
