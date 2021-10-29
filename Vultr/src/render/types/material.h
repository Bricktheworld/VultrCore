// TODO: Reimplement color
#pragma once
#include <functional>
#include <types/types.h>
#include "shader.h"
#include <filesystem/file.h>

namespace Vultr
{
    struct Color
    {
        f64 red;
        f64 green;
        f64 blue;
        f64 alpha;
    };
#define MAX_UNIFORMS 16
#define MAX_MATERIAL_TEXTURES 8
    struct MaterialUniform
    {

        union Data {
            bool u_bool;
            u32 u_u32;
            s32 u_s32;
            f32 u_f32;
            Vec2 u_vec2;
            Vec3 u_vec3;
            Vec4 u_vec4;
            Color u_color;
            Mat4 u_mat4;
            Data()
            {
                memset(this, 0, sizeof(Data));
            }
            ~Data(){};
        } data;

        enum Type : u8
        {
            BOOL = 0,
            U32 = 1,
            S32 = 2,
            F32 = 3,
            VEC2 = 4,
            VEC3 = 5,
            VEC4 = 6,
            COLOR = 7,
            MAT4 = 8,
            EMPTY = 9,
        };

        Type type = EMPTY;
        char *location;
    };
    // typedef std::function<void(Shader)> MaterialBindCallback;

    struct Material
    {
        struct TextureResource
        {
            TextureSource file;
            char *location = nullptr;
            char *is_set_location = nullptr;
        };

        MaterialUniform uniforms[MAX_UNIFORMS];
        size_t uniform_count = 0;

        ShaderSource shader_source;

        TextureResource textures[MAX_MATERIAL_TEXTURES];
        size_t texture_count = 0;

        // If you need more than 16 uniform locations or if you have something like a struct, then just use this callback
        // MaterialBindCallback bind_callback;
    };

    void bool_uniform(Material &material, const char *location, bool value);
    void u32_uniform(Material &material, const char *location, u32 value);
    void s32_uniform(Material &material, const char *location, s32 value);
    void f32_uniform(Material &material, const char *location, f32 value);
    void vec2_uniform(Material &material, const char *location, Vec2 value);
    void vec3_uniform(Material &material, const char *location, Vec3 value);
    void vec4_uniform(Material &material, const char *location, Vec4 value);
    void color_uniform(Material &material, const char *location, Color value);
    void mat4_uniform(Material &material, const char *location, Mat4 value);
    void texture_uniform(Material &material, const char *location, const TextureSource &source, const char *is_set_location = "");

    void material_bind_uniforms(const Material &material, Shader *shader);

    // void to_json(json &j, const Material &m);
    // void from_json(const json &j, Material &m);
} // namespace Vultr
