// TODO: Reimplement using custom VTL
#include <render/types/material.h>
#include <glm/gtc/type_ptr.hpp>

namespace Vultr
{
#define CAN_ADD_UNIFORM(m) assert(m.uniform_count < MAX_UNIFORMS && "Max uniforms added!");

    void bool_uniform(Material &material, const char *location, bool value)
    {
        CAN_ADD_UNIFORM(material)
        auto &uniform = material.uniforms[material.uniform_count];
        uniform.type = MaterialUniform::BOOL;
        uniform.data.u_bool = value;
        // uniform.location = str(location);
        material.uniform_count++;
    }
    void u32_uniform(Material &material, const char *location, u32 value)
    {
        CAN_ADD_UNIFORM(material)
        auto &uniform = material.uniforms[material.uniform_count];
        uniform.type = MaterialUniform::U32;
        uniform.data.u_u32 = value;
        // uniform.location = str(location);
        material.uniform_count++;
    }
    void s32_uniform(Material &material, const char *location, s32 value)
    {
        CAN_ADD_UNIFORM(material)
        auto &uniform = material.uniforms[material.uniform_count];
        uniform.type = MaterialUniform::S32;
        uniform.data.u_s32 = value;
        // uniform.location = str(location);
        material.uniform_count++;
    }
    void f32_uniform(Material &material, const char *location, f32 value)
    {
        CAN_ADD_UNIFORM(material)
        auto &uniform = material.uniforms[material.uniform_count];
        uniform.type = MaterialUniform::F32;
        uniform.data.u_f32 = value;
        // uniform.location = str(location);
        material.uniform_count++;
    }
    void vec2_uniform(Material &material, const char *location, Vec2 value)
    {
        CAN_ADD_UNIFORM(material)
        auto &uniform = material.uniforms[material.uniform_count];
        uniform.type = MaterialUniform::VEC2;
        uniform.data.u_vec2 = value;
        // uniform.location = str(location);
        material.uniform_count++;
    }
    void vec3_uniform(Material &material, const char *location, Vec3 value)
    {
        CAN_ADD_UNIFORM(material)
        auto &uniform = material.uniforms[material.uniform_count];
        uniform.type = MaterialUniform::VEC3;
        uniform.data.u_vec3 = value;
        // uniform.location = str(location);
        material.uniform_count++;
    }
    void vec4_uniform(Material &material, const char *location, Vec4 value)
    {
        CAN_ADD_UNIFORM(material)
        auto &uniform = material.uniforms[material.uniform_count];
        uniform.type = MaterialUniform::VEC4;
        uniform.data.u_vec4 = value;
        // uniform.location = str(location);
        material.uniform_count++;
    }

    void color_uniform(Material &material, const char *location, Color value)
    {
        CAN_ADD_UNIFORM(material)
        auto &uniform = material.uniforms[material.uniform_count];
        uniform.type = MaterialUniform::COLOR;
        uniform.data.u_color = value;
        // uniform.location = str(location);
        material.uniform_count++;
    }

    void mat4_uniform(Material &material, const char *location, Mat4 value)
    {
        CAN_ADD_UNIFORM(material)
        auto &uniform = material.uniforms[material.uniform_count];
        uniform.type = MaterialUniform::MAT4;
        uniform.data.u_mat4 = value;
        // uniform.location = str(location);
        material.uniform_count++;
    }

    void texture_uniform(Material &material, const char *location, const TextureSource &source, const char *is_set_location)
    {
        assert(material.texture_count < MAX_MATERIAL_TEXTURES && "Max textures added!");
        auto &texture = material.textures[material.texture_count];
        // texture.location = str(location);
        texture.file = source;
        // texture.is_set_location = str(is_set_location);
        material.texture_count++;
    }

    void material_bind_uniforms(const Material &material, Shader *shader)
    {
        for (s16 i = 0; i < material.uniform_count; i++)
        {
            auto &uniform = material.uniforms[i];
            auto &location = uniform.location;
            auto &data = uniform.data;
            switch (uniform.type)
            {
                case MaterialUniform::BOOL:
                    set_uniform_1i(shader, location, data.u_bool ? 1 : 0);
                    break;
                case MaterialUniform::U32:
                    set_uniform_1i(shader, location, data.u_u32);
                    break;
                case MaterialUniform::S32:
                    set_uniform_1i(shader, location, data.u_s32);
                    break;
                case MaterialUniform::F32:
                    set_uniform_1f(shader, location, data.u_f32);
                    break;
                case MaterialUniform::VEC2:
                    set_uniform_2f(shader, location, data.u_vec2);
                    break;
                case MaterialUniform::VEC3:
                    set_uniform_3f(shader, location, data.u_vec3);
                    break;
                case MaterialUniform::VEC4:
                    set_uniform_4f(shader, location, data.u_vec4);
                    break;
                case MaterialUniform::COLOR:
                    set_uniform_4f(shader, location, Vec4(data.u_color.red, data.u_color.green, data.u_color.blue, data.u_color.alpha) / Vec4(255));
                    break;
                case MaterialUniform::MAT4:
                    set_uniform_matrix_4fv(shader, location, glm::value_ptr(data.u_mat4));
                    break;
                case MaterialUniform::EMPTY:
                    // Nothing to do here, as this is the default
                    break;
            }
        }
        for (s16 i = 0; i < material.texture_count; i++)
        {
            auto &texture = material.textures[i];

            set_uniform_1i(shader, texture.location, i);
            // if (!strequal(texture.is_set_location, ""))
            // {
            //     if (texture.file != TextureSource(""))
            //     {
            //         set_uniform_1i(shader, texture.is_set_location, 1);
            //     }
            //     else
            //     {
            //         set_uniform_1i(shader, texture.is_set_location, 0);
            //     }
            // }
        }
    }
} // namespace Vultr
