#pragma once
#include <types/types.h>
#include <glad/glad.h>

namespace Vultr
{
	struct Texture
	{
		u32 id              = 0;
		u32 width           = 0;
		u32 height          = 0;
		u32 level           = 0;
		u32 type            = GL_TEXTURE_2D;
		u32 internal_format = GL_RGB;
		u32 format          = GL_RGB;
		u32 pixel_data_type = GL_UNSIGNED_BYTE;
		unsigned char *data = nullptr;

		Texture()           = default;
		Texture(u32 type);
		Texture(const Texture &other) = delete;
	};

	void generate_texture(Texture *texture, GLenum type);

#define invalid_texture()                                                                                                                                                                                             \
	Texture {}
	void texture_image_2D(Texture *texture, u32 level, GLenum internalformat, u32 width, u32 height, GLenum format, GLenum type, const void *data);
	void texture_image_2D(GLenum target, u32 level, GLenum internalformat, u32 width, u32 height, GLenum format, GLenum type, const void *data);
	void texture_parameter_i(Texture *texture, GLenum pname, u32 param);

	bool is_valid_texture(Texture *texture);
	void delete_texture(Texture *texture);

	void bind_texture(Texture *texture, GLenum slot);
	void unbind_texture(Texture *texture);
} // namespace Vultr
