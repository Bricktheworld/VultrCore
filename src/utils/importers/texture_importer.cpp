// TODO: Reimplement using custom hashtable implementation
#include <filesystem/resource_manager.h>
#include <filesystem/importers/texture_importer.h>
#include <stb_image/stb_image.h>

namespace Vultr
{
	template <>
	bool load_resource<Texture>(const VirtualFilesystem *vfs, VFileHandle file, Texture *resource, ResourceQueueItem *item)
	{
		// assert(vfs_file_exists(vfs, file) && "Cannot load texture, file does not exist!");
		// const char *path = vfs->file_table_path.at(file).path;
		// printf("Loading texture %s\n", path);

		// VFileStream *stream = vfs_open(vfs, file, "rb");

		// u64 size = 0;
		// auto *buf = vfs_read_full(vfs, &size, stream);
		// vfs_close(stream);

		// if (buf == nullptr)
		// {
		//     fprintf(stderr, "Failed to load texture %s! Something went wrong opening the file...\n", path);
		//     return false;
		// }

		// unsigned char *loaded_buf = TextureImporter::texture_load_memory(resource, buf, size);
		// vfs_free_buf(buf);
		// if (loaded_buf == nullptr)
		// {
		//     stbi_image_free(loaded_buf);
		//     fprintf(stderr, "Failed to load texture %s! Something went wrong loading into memory...\n", path);
		//     return false;
		// }

		// item->type = ResourceType::TEXTURE;
		// item->file = file;
		// item->temp_buf = loaded_buf;

		// return true;
	}

	template <>
	bool finalize_resource<Texture>(VFileHandle file, Texture *data, void *buffer)
	{
		printf("Finalizing texture on main thread!\n");

		auto *buf = static_cast<unsigned char *>(buffer);

		TextureImporter::texture_load_gpu(data, buf);

		stbi_image_free(buf);

		return true;
	}

	template <>
	void free_resource<Texture>(Texture *resource)
	{
		delete_texture(resource);
	}
	namespace TextureImporter
	{
		bool texture_import(Texture *texture, const TextureSource *source)
		{
			u8 *buf = texture_load_file(texture, source);
			if (buf == nullptr)
				return false;

			texture_load_gpu(texture, buf);

			stbi_image_free(buf);
			return true;
		}

		bool texture_import(Texture *texture, const unsigned char *data, u64 size)
		{
			auto *buf = texture_load_memory(texture, data, size);
			if (buf == nullptr)
				return false;

			texture_load_gpu(texture, buf);

			stbi_image_free(buf);
			return true;
		}

		unsigned char *texture_load_file(Texture *texture, const TextureSource *source)
		{
			stbi_set_flip_vertically_on_load(1);
			s32 width;
			s32 height;
			s32 bpp;
			u8 *buffer = stbi_load(source->path, &width, &height, &bpp, 4);

			if (buffer == nullptr)
			{
				return nullptr;
			}

			texture->width  = width;
			texture->height = height;

			return buffer;
		}

		unsigned char *texture_load_memory(Texture *texture, const unsigned char *data, u64 size)
		{
			s32 width  = 0;
			s32 height = 0;
			u8 *buffer = stbi_load_from_memory(data, size, &width, &height, nullptr, 4);
			if (buffer == nullptr)
			{
				return nullptr;
			}

			texture->width  = width;
			texture->height = height;
			return buffer;
		}

		void texture_load_gpu(Texture *texture, const unsigned char *data)
		{
			bind_texture(texture, GL_TEXTURE0);
			texture_parameter_i(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			texture_parameter_i(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			texture_parameter_i(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			texture_parameter_i(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			texture_image_2D(texture, 0, GL_SRGB_ALPHA, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		// bool TextureImporter::import_skybox(Texture *texture, const std::vector<TextureSource> &paths)
		// {
		//     assert(texture->type == GL_TEXTURE_CUBE_MAP && "Can only write to texture marked as a cube map");

		//     stbi_set_flip_vertically_on_load(0);
		//     bind_texture(texture, GL_TEXTURE0);

		//     int width, height, nrChannels;
		//     for (unsigned int i = 0; i < paths.size(); i++)
		//     {
		//         unsigned char *data = stbi_load(paths[i].path, &width, &height, &nrChannels, 0);
		//         if (data)
		//         {
		//             texture_image_2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
		//             stbi_image_free(data);
		//         }
		//         else
		//         {
		//             std::cout << "Cubemap texture failed to load at path: " << paths[i].path << std::endl;
		//             stbi_image_free(data);
		//             return false;
		//         }
		//     }
		//     texture_parameter_i(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//     texture_parameter_i(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//     texture_parameter_i(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//     texture_parameter_i(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//     texture_parameter_i(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//     return true;
		// }
	} // namespace TextureImporter

} // namespace Vultr
