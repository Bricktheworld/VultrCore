// TODO: Reimplement using custom VTL
#pragma once
#include <render/types/shader.h>
#include <string>
#include <types/types.h>
#include <filesystem/file.h>

namespace Vultr::ShaderImporter
{
    enum struct ShaderType : u8
    {
        NONE = 0xFF,
        VERTEX = 0,
        FRAGMENT = 1
    };
    struct ShaderProgramSource
    {
        char *vert_src = nullptr;
        char *frag_src = nullptr;

        ~ShaderProgramSource()
        {
            if (vert_src != nullptr)
                free(vert_src);
            if (frag_src != nullptr)
                free(frag_src);
        }
    };
    bool shader_import(Shader *shader, const ShaderSource *source);
    bool shader_import_file(ShaderProgramSource *result, const ShaderSource *source);
    bool shader_import_memory(ShaderProgramSource *result, const unsigned char *data, u64 len);
    bool shader_load_gpu(Shader *shader, const ShaderProgramSource *source);

    u32 shader_compile(u32 type, const char *src);

    // const ShaderProgramSource OUTLINE = {
    //     .vert_src = str("layout (location = 0) in vec3 position;\n"
    //                     "layout (location = 1) in vec3 normal;\n"
    //                     "layout (location = 2) in vec2 uv; \n"
    //                     "out vec3 FragPos; \n"
    //                     "uniform mat4 model; \n"
    //                     "uniform mat4 view; \n"
    //                     "uniform mat4 projection; \n"
    //                     ""
    //                     "void main() \n"
    //                     "{\n"
    //                     "   gl_Position = projection * view * model * vec4(position, 1.0f); \n"
    //                     "   FragPos = vec3(model * vec4(position, 1.0f)); \n"
    //                     ""
    //                     ""
    //                     "}\n"),

    //     .frag_src = str("layout (location = 0) in vec3 FragPos;\n"
    //                     "out vec4 FragColor; \n"
    //                     "uniform vec4 color; \n"
    //                     ""
    //                     "void main() \n"
    //                     "{\n"
    //                     "   FragColor = color; \n"
    //                     ""
    //                     ""
    //                     "}\n"),
    // };
    // const ShaderProgramSource EDITOR_INPUT = {
    //     .vert_src = str("#version 330 core\n"
    //                     "#extension GL_ARB_separate_shader_objects: enable\n"
    //                     "layout (location = 0) in vec3 position;\n"
    //                     "layout (location = 1) in vec3 normal;\n"
    //                     "layout (location = 2) in vec2 uv; \n"
    //                     "out vec3 FragPos; \n"
    //                     "uniform mat4 model; \n"
    //                     "uniform mat4 view; \n"
    //                     "uniform mat4 projection; \n"
    //                     ""
    //                     "void main() \n"
    //                     "{\n"
    //                     "   gl_Position = projection * view * model * vec4(position, 1.0f); \n"
    //                     "   FragPos = vec3(model * vec4(position, 1.0f)); \n"
    //                     ""
    //                     ""
    //                     "}\n"),
    //     .frag_src = str("#version 330 core\n"
    //                     "#extension GL_ARB_separate_shader_objects: enable\n"
    //                     "layout (location = 0) in vec3 FragPos;\n"
    //                     "out vec4 FragColor; \n"
    //                     "uniform vec4 color; \n"
    //                     ""
    //                     "void main() \n"
    //                     "{\n"
    //                     "   FragColor = color; \n"
    //                     ""
    //                     ""
    //                     "}\n"),
    // };

} // namespace Vultr::ShaderImporter
