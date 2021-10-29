#include <glm/gtc/type_ptr.hpp>
#include <render/types/shader.h>
#include <glad/glad.h>

namespace Vultr
{
    void bind_shader(Shader *shader)
    {
        assert(shader->id != 0 && "Invalid shader!");
        glUseProgram(shader->id);
    }

    void unbind_all_shaders()
    {
        glUseProgram(0);
    }

    bool is_valid_shader(Shader *shader)
    {
        return shader->id > 0;
    }

    void delete_shader(Shader *shader)
    {
        glDeleteProgram(shader->id);
        shader->id = 0;
    }

    void set_uniform_matrix_4fv(u32 location, const float *value)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, value);
    }

    void set_uniform_matrix_3fv(u32 location, const float *value)
    {
        glUniformMatrix3fv(location, 1, GL_FALSE, value);
    }

    void set_uniform_4f(u32 location, const Vec4 &value)
    {
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }
    void set_uniform_3f(u32 location, const Vec3 &value)
    {
        glUniform3f(location, value.x, value.y, value.z);
    }
    void set_uniform_2f(u32 location, const Vec2 &value)
    {
        glUniform2f(location, value.x, value.y);
    }
    void set_uniform_1i(u32 location, s32 value)
    {
        glUniform1i(location, value);
    }
    void set_uniform_1iv(u32 location, size_t size, const s32 *value)
    {
        glUniform1iv(location, size, value);
    }
    void set_uniform_1f(u32 location, f32 value)
    {
        glUniform1f(location, value);
    }

    void set_uniform_bool(u32 location, bool value)
    {
        glUniform1i(location, value ? 1 : 0);
    }

    void set_uniform_matrix_4fv(Shader *shader, const char *uniform, const float *value)
    {
        set_uniform_matrix_4fv(get_uniform_location(shader, uniform), value);
    }

    void set_uniform_matrix_3fv(Shader *shader, const char *uniform, const float *value)
    {
        set_uniform_matrix_3fv(get_uniform_location(shader, uniform), value);
    }

    void set_uniform_4f(Shader *shader, const char *uniform, const Vec4 &value)
    {
        set_uniform_4f(get_uniform_location(shader, uniform), value);
    }
    void set_uniform_3f(Shader *shader, const char *uniform, const Vec3 &value)
    {
        set_uniform_3f(get_uniform_location(shader, uniform), value);
    }
    void set_uniform_2f(Shader *shader, const char *uniform, const Vec2 &value)
    {
        set_uniform_2f(get_uniform_location(shader, uniform), value);
    }
    void set_uniform_1i(Shader *shader, const char *uniform, s32 value)
    {
        set_uniform_1i(get_uniform_location(shader, uniform), value);
    }
    void set_uniform_1iv(Shader *shader, const char *uniform, size_t size, const s32 *value)
    {
        set_uniform_1iv(get_uniform_location(shader, uniform), size, value);
    }
    void set_uniform_1f(Shader *shader, const char *uniform, f32 value)
    {
        set_uniform_1f(get_uniform_location(shader, uniform), value);
    }

    u32 get_uniform_location(Shader *shader, const char *uniform)
    {
        return glGetUniformLocation(shader->id, uniform);
    }
} // namespace Vultr
