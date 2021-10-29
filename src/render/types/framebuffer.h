#pragma once
#include <types/types.h>
#include "texture.h"
#include <glm/glm.hpp>

// For memset
#include <cstring>

namespace Vultr
{

    typedef u32 Renderbuffer;

    bool is_valid_renderbuffer(Renderbuffer rbo);
    Renderbuffer new_renderbuffer();
    void delete_renderbuffer(Renderbuffer *rbo);

    void bind_renderbuffer(Renderbuffer rbo);
    void unbind_all_renderbuffers();

    namespace Internal
    {
        // In a framebuffer, you can either attach a texture or a renderbuffer
        struct TextureOrRBO
        {
            enum Type
            {
                RENDERBUFFER,
                TEXTURE,
                INVALID,
            };

            Type type;

            union TextureOrRBOData {
                Texture *texture;
                Renderbuffer rbo;

                TextureOrRBOData()
                {
                    memset(this, 0, sizeof(TextureOrRBOData));
                }
                ~TextureOrRBOData(){};
            } data;
        };
    } // namespace Internal

    struct Framebuffer
    {
        u32 id = 0;

        u32 width = 0;
        u32 height = 0;

        Internal::TextureOrRBO *color_attachments = nullptr;
        s32 max_color_attachments = 0;

        Internal::TextureOrRBO depth_stencil_attachment = {};
        Framebuffer() = default;
        Framebuffer(u32 width, u32 height);
    };

#define invalid_framebuffer()                                                                                                                                                                                         \
    Framebuffer                                                                                                                                                                                                       \
    {                                                                                                                                                                                                                 \
    }

    void new_framebuffer(Framebuffer *fbo, u32 width, u32 height);

    void delete_framebuffer(Framebuffer *fbo);

    Vec2 get_framebuffer_dimensions(const Framebuffer *fbo);

    bool is_valid_framebuffer(const Framebuffer *fbo);
    bool confirm_complete_framebuffer();

    Texture *get_framebuffer_color_texture(Framebuffer *fbo, s16 slot);
    Texture *get_framebuffer_depth_stencil_texture(Framebuffer *fbo, s16 slot);

    void attach_color_texture_framebuffer(Framebuffer *fbo, Texture *texture, s16 slot, GLenum internal_format, GLenum format, GLenum data_type);
    void attach_color_renderbuffer_framebuffer(Framebuffer *fbo, Renderbuffer rbo, s16 slot);
    void remove_color_attachment_framebuffer(Framebuffer *fbo, s16 slot, bool dealloc = true);

    void attach_stencil_depth_texture_framebuffer(Framebuffer *fbo, Texture *texture);
    void attach_stencil_depth_renderbuffer_framebuffer(Framebuffer *fbo, Renderbuffer rbo);
    void remove_stencil_depth_attachment_framebuffer(Framebuffer *fbo, bool dealloc = true);

    void recreate_framebuffer(Framebuffer *fbo);
    void resize_framebuffer(Framebuffer *fbo, u32 width, u32 height);

    void bind_framebuffer(const Framebuffer *fbo, GLenum mode = GL_FRAMEBUFFER);
    void unbind_all_framebuffers(GLenum mode = GL_FRAMEBUFFER);

} // namespace Vultr
