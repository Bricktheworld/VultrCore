#include <render/types/framebuffer.h>

namespace Vultr
{
	using namespace Internal;

	static TextureOrRBO invalid_texture_or_rbo()
	{
		return {
			.type = TextureOrRBO::INVALID,
		};
	}

	static bool is_valid_texture_or_rbo(TextureOrRBO *t_or_rbo)
	{
		if (t_or_rbo->type == TextureOrRBO::INVALID)
			return false;

		if (t_or_rbo->type == TextureOrRBO::TEXTURE)
		{
			return is_valid_texture(t_or_rbo->data.texture);
		}
		else
		{
			return is_valid_renderbuffer(t_or_rbo->data.rbo);
		}
	}

	static void delete_texture_or_rbo(TextureOrRBO *t_or_rbo)
	{
		if (t_or_rbo->type == TextureOrRBO::TEXTURE)
		{
			delete_texture(t_or_rbo->data.texture);
		}
		else
		{
			delete_renderbuffer(&t_or_rbo->data.rbo);
		}
		*t_or_rbo = invalid_texture_or_rbo();
	}

	static TextureOrRBO new_texture_object(Texture *texture)
	{
		TextureOrRBO t_or_rbo = {
			.type = TextureOrRBO::TEXTURE,
		};
		t_or_rbo.data.texture = texture;
		return t_or_rbo;
	}

	static TextureOrRBO new_renderbuffer_object(Renderbuffer rbo)
	{
		TextureOrRBO t_or_rbo = {
			.type = TextureOrRBO::RENDERBUFFER,
		};
		t_or_rbo.data.rbo = rbo;
		return t_or_rbo;
	}

	bool is_valid_renderbuffer(Renderbuffer rbo) { return rbo > 0; }

	Renderbuffer new_renderbuffer()
	{
		Renderbuffer rbo;

		// TODO: See if it is possible to use glgen instead of create
		glCreateRenderbuffers(1, &rbo);
		return rbo;
	}

	void delete_renderbuffer(Renderbuffer *rbo)
	{
		glDeleteRenderbuffers(1, rbo);
		rbo = 0;
	}

	void bind_renderbuffer(Renderbuffer rbo) { glBindRenderbuffer(GL_RENDERBUFFER, rbo); }

	void unbind_all_renderbuffers() { glBindRenderbuffer(GL_RENDERBUFFER, 0); }

	void new_framebuffer(Framebuffer *fbo, u32 width, u32 height)
	{
		glGenFramebuffers(1, &fbo->id);

		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &fbo->max_color_attachments);

		fbo->color_attachments = new TextureOrRBO[fbo->max_color_attachments];
		for (s16 i = 0; i < fbo->max_color_attachments; i++)
		{
			fbo->color_attachments[i] = invalid_texture_or_rbo();
		}
		fbo->depth_stencil_attachment = invalid_texture_or_rbo();

		fbo->width                    = width;
		fbo->height                   = height;
	}

	Framebuffer::Framebuffer(u32 width, u32 height) {}

	void delete_framebuffer(Framebuffer *fbo)
	{
		assert(is_valid_framebuffer(fbo) && "Invalid framebuffer!");
		glDeleteFramebuffers(1, &fbo->id);
		fbo->id = 0;
		for (s16 i = 0; i < fbo->max_color_attachments; i++)
		{
			if (is_valid_texture_or_rbo(&fbo->color_attachments[i]))
			{
				delete_texture_or_rbo(&fbo->color_attachments[i]);
			}
		}
		if (is_valid_texture_or_rbo(&fbo->depth_stencil_attachment))
		{
			delete_texture_or_rbo(&fbo->depth_stencil_attachment);
		}
		delete fbo->color_attachments;
	}

	Vec2 get_framebuffer_dimensions(const Framebuffer *fbo) { return Vec2(fbo->width, fbo->height); }

	bool is_valid_framebuffer(const Framebuffer *fbo) { return fbo->id > 0; }

	bool confirm_complete_framebuffer()
	{
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			fprintf(stderr, "Framebuffer is not complete!\n");
			return false;
		}
		else
		{
			return true;
		}
	}

	Texture *get_framebuffer_color_texture(Framebuffer *fbo, s16 slot)
	{
		auto *t_or_rbo = &fbo->color_attachments[slot];
		assert(is_valid_texture_or_rbo(t_or_rbo) && "Texture is not attached to color slot!");
		assert(t_or_rbo->type == TextureOrRBO::TEXTURE && "Slot is a renderbuffer, not a texture!");
		return t_or_rbo->data.texture;
	}

	Texture *get_framebuffer_depth_stencil_texture(Framebuffer *fbo, s16 slot)
	{
		auto *t_or_rbo = &fbo->depth_stencil_attachment;
		assert(is_valid_texture_or_rbo(t_or_rbo) && "Texture is not attached depth-stencil!");
		assert(t_or_rbo->type == TextureOrRBO::TEXTURE && "Depth-stencil is a renderbuffer, not a texture!");
		return t_or_rbo->data.texture;
	}

	static bool can_attach_color_texture(Framebuffer *fbo, s16 slot) { return slot < fbo->max_color_attachments; }

	void attach_color_texture_framebuffer(Framebuffer *fbo, Texture *texture, s16 slot, GLenum internal_format, GLenum format, GLenum data_type)
	{
		assert(is_valid_framebuffer(fbo) && "Invalid framebuffer!");
		assert(!is_valid_texture_or_rbo(&fbo->color_attachments[slot]) && "Slot is already taken!");
		assert(can_attach_color_texture(fbo, slot) && "Not enough space for texture!");
		assert(texture->type == GL_TEXTURE_2D && "Only a texture 2D can be color attached to a framebuffer");
		bind_framebuffer(fbo);

		bind_texture(texture, GL_TEXTURE0);
		texture->width  = fbo->width;
		texture->height = fbo->height;

		texture_image_2D(texture, 0, internal_format, texture->width, texture->height, format, data_type, nullptr);

		texture_parameter_i(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		texture_parameter_i(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		texture_parameter_i(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		texture_parameter_i(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		texture_parameter_i(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, texture->type, texture->id, 0);

		fbo->color_attachments[slot] = new_texture_object(texture);
	}

	void attach_color_renderbuffer_framebuffer(Framebuffer *fbo, Renderbuffer rbo, s16 slot)
	{
		assert(is_valid_framebuffer(fbo) && "Invalid framebuffer!");
		assert(!is_valid_texture_or_rbo(&fbo->color_attachments[slot]) && "Slot is already taken!");
		assert(can_attach_color_texture(fbo, slot) && "Not enough space for texture!");

		bind_framebuffer(fbo);

		bind_renderbuffer(rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, fbo->width, fbo->height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_RENDERBUFFER, rbo);

		fbo->color_attachments[slot] = new_renderbuffer_object(rbo);
	}

	void remove_color_attachment_framebuffer(Framebuffer *fbo, s16 slot, bool dealloc)
	{
		assert(can_attach_color_texture(fbo, slot) && "Invalid slot!");

		bind_framebuffer(fbo);

		auto *t_or_rbo = &fbo->color_attachments[slot];

		assert(is_valid_texture_or_rbo(t_or_rbo) && "No texture or renderbuffer attached to the fbo in this slot!");

		if (t_or_rbo->type == TextureOrRBO::TEXTURE)
		{
			auto &texture = t_or_rbo->data.texture;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, texture->type, 0, 0);
		}
		else if (t_or_rbo->type == TextureOrRBO::RENDERBUFFER)
		{
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_RENDERBUFFER, 0);
		}

		if (dealloc)
		{
			Vultr::delete_texture_or_rbo(t_or_rbo);
		}

		*t_or_rbo = invalid_texture_or_rbo();
	}

	void attach_stencil_depth_texture_framebuffer(Framebuffer *fbo, Texture *texture)
	{
		assert(is_valid_framebuffer(fbo) && "Invalid framebuffer!");
		assert(texture->type == GL_TEXTURE_2D && "Only a texture 2D can be depth-stencil attached to a framebuffer");
		assert(!is_valid_texture_or_rbo(&fbo->depth_stencil_attachment) && "Depth-stencil attachment already taken!");

		bind_framebuffer(fbo);

		bind_texture(texture, GL_TEXTURE0);

		texture->width  = fbo->width;
		texture->height = fbo->height;

		texture_image_2D(texture, 0, GL_DEPTH24_STENCIL8, texture->width, texture->height, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

		texture_parameter_i(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		texture_parameter_i(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		texture_parameter_i(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		texture_parameter_i(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		texture_parameter_i(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, texture->type, texture->id, 0);

		fbo->depth_stencil_attachment = new_texture_object(texture);
	}

	void attach_stencil_depth_renderbuffer_framebuffer(Framebuffer *fbo, Renderbuffer rbo)
	{
		assert(is_valid_framebuffer(fbo) && "Invalid framebuffer!");
		assert(!is_valid_texture_or_rbo(&fbo->depth_stencil_attachment) && "Depth-stencil attachment already taken!");

		bind_framebuffer(fbo);

		bind_renderbuffer(rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbo->width, fbo->height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		fbo->depth_stencil_attachment = new_renderbuffer_object(rbo);
	}

	void remove_stencil_depth_attachment_framebuffer(Framebuffer *fbo, bool dealloc)
	{

		bind_framebuffer(fbo);
		auto *t_or_rbo = &fbo->depth_stencil_attachment;

		assert(is_valid_texture_or_rbo(t_or_rbo) && "No depth-stencil attachment for framebuffer");

		if (t_or_rbo->type == TextureOrRBO::TEXTURE)
		{
			auto &texture = t_or_rbo->data.texture;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, texture->type, 0, 0);
		}
		else if (t_or_rbo->type == TextureOrRBO::RENDERBUFFER)
		{
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
		}

		if (dealloc)
		{
			Vultr::delete_texture_or_rbo(t_or_rbo);
		}

		*t_or_rbo = invalid_texture_or_rbo();
	}

	void recreate_framebuffer(Framebuffer *old_fbo)
	{
		Framebuffer fbo;
		new_framebuffer(&fbo, old_fbo->width, old_fbo->height);

		for (u16 i = 0; i < fbo.max_color_attachments; i++)
		{
			auto *t_or_rbo = &old_fbo->color_attachments[i];
			if (is_valid_texture_or_rbo(t_or_rbo))
			{
				if (t_or_rbo->type == TextureOrRBO::TEXTURE)
				{
					auto *old_texture = t_or_rbo->data.texture;
					Texture texture;
					generate_texture(&texture, old_texture->type);
					old_texture->id = texture.id;

					attach_color_texture_framebuffer(&fbo, old_texture, i, texture.internal_format, texture.format, texture.pixel_data_type);
				}
				else
				{
					auto rbo = new_renderbuffer();

					bind_renderbuffer(rbo);
					attach_color_renderbuffer_framebuffer(&fbo, rbo, i);
				}
			}
		}

		auto *t_or_rbo = &old_fbo->depth_stencil_attachment;
		if (is_valid_texture_or_rbo(t_or_rbo))
		{
			if (t_or_rbo->type == TextureOrRBO::TEXTURE)
			{
				auto *old_texture = t_or_rbo->data.texture;
				Texture texture;
				generate_texture(&texture, old_texture->type);
				old_texture->id = texture.id;

				bind_texture(old_texture, GL_TEXTURE0);
				attach_stencil_depth_texture_framebuffer(&fbo, old_texture);
			}
			else
			{
				auto rbo = new_renderbuffer();

				bind_renderbuffer(rbo);
				attach_stencil_depth_renderbuffer_framebuffer(&fbo, rbo);
			}
		}

		confirm_complete_framebuffer();

		delete_framebuffer(old_fbo);
		*old_fbo = fbo;
	}

	void resize_framebuffer(Framebuffer *fbo, u32 width, u32 height)
	{
		// If the dimensions already match then there is nothing to do
		if (fbo->width == width && fbo->height == height)
			return;

		fbo->width  = width;
		fbo->height = height;
		recreate_framebuffer(fbo);
	}

	void bind_framebuffer(const Framebuffer *fbo, GLenum mode)
	{
		assert(is_valid_framebuffer(fbo) && "Invalid framebuffer!");
		glBindFramebuffer(mode, fbo->id);
	}

	void unbind_all_framebuffers(GLenum mode) { glBindFramebuffer(mode, 0); }
} // namespace Vultr
