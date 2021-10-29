// TODO: Reimplement in custom VTL
#include <filesystem/resource_manager.h>
#include <filesystem/importers/shader_importer.h>
#include <stdio.h>

namespace Vultr
{
    template <>
    bool load_resource<Shader>(const VirtualFilesystem *vfs, VFileHandle file, Shader *resource, ResourceQueueItem *item)
    {
        // assert(vfs_file_exists(vfs, file) && "Cannot load shader, file does not exist!");
        // const char *path = vfs->file_table_path.at(file).path;
        // printf("Loading shader %s\n", path);

        // VFileStream *stream = vfs_open(vfs, file, "rb");

        // u64 size = 0;
        // auto *buf = vfs_read_full(vfs, &size, stream);
        // vfs_close(stream);

        // if (buf == nullptr)
        // {
        //     fprintf(stderr, "Failed to load shader %s! Something went wrong opening the file...\n", path);
        //     return false;
        // }

        // auto *source = new ShaderImporter::ShaderProgramSource();
        // bool res = ShaderImporter::shader_import_memory(source, buf, size);
        // vfs_free_buf(buf);

        // if (!res)
        // {
        //     fprintf(stderr, "Failed to load shader %s! Something went wrong loading into memory...\n", path);
        //     return false;
        // }

        // item->type = ResourceType::SHADER;
        // item->file = file;
        // item->temp_buf = source;

        // return true;
    }

    template <>
    bool finalize_resource<Shader>(VFileHandle file, Shader *data, void *buffer)
    {
        printf("Finalizing texture on main thread!\n");

        auto *source = static_cast<ShaderImporter::ShaderProgramSource *>(buffer);
        bool res = ShaderImporter::shader_load_gpu(data, source);
        delete source;

        if (!res)
        {
            fprintf(stderr, "Failed to load shader onto the GPU! This is often caused by a compilation error, however in rare circumstances this could also be a bug with the engine...\n");
            return false;
        }

        return true;
    }

    template <>
    void free_resource<Shader>(Shader *resource)
    {
        delete_shader(resource);
    }

    namespace ShaderImporter
    {
        bool shader_import_memory(ShaderProgramSource *result, const unsigned char *_data, u64 len)
        {
            // NOTE (Brandon): While this technically is not very good, it's fine because we are only reading ascii characters in the shader, so none of the data is lost in the conversion
            const char *data = reinterpret_cast<const char *>(_data);

            ShaderType type = ShaderType::NONE;

            const char *vert_line_identifier = "#shader vertex";
            const size_t vert_line_len = strlen(vert_line_identifier);
            const char *frag_line_identifier = "#shader fragment";
            const size_t frag_line_len = strlen(frag_line_identifier);

            // NOTE(Brandon): These are oversized, but there really is no point in trying to resize them properly since they are temporary and the memory difference is so negligble.
            // result->vert_src = str(len - 1);
            // char *vert_ptr = result->vert_src;

            // result->frag_src = str(len - 1);
            // char *frag_ptr = result->frag_src;

            // for (u64 i = 0; i < len; i++)
            // {
            //     if (i < len - vert_line_len || i < len - frag_line_len)
            //     {
            //         if (strnequal(data + i, vert_line_identifier, vert_line_len))
            //         {
            //             type = ShaderType::VERTEX;
            //             i += vert_line_len - 1;
            //             continue;
            //         }
            //         else if (strnequal(data + i, frag_line_identifier, frag_line_len))
            //         {
            //             type = ShaderType::FRAGMENT;
            //             i += frag_line_len - 1;
            //             continue;
            //         }
            //     }
            //     if (type == ShaderType::VERTEX)
            //     {
            //         *vert_ptr = data[i];
            //         vert_ptr++;
            //     }
            //     else if (type == ShaderType::FRAGMENT)
            //     {
            //         *frag_ptr = data[i];
            //         frag_ptr++;
            //     }
            // }
            // // Something went wrong if we never advanced these pointers
            // if (vert_ptr == result->vert_src)
            //     return false;
            // if (frag_ptr == result->frag_src)
            //     return false;

            // return true;
        }

        bool shader_import_file(ShaderProgramSource *result, const ShaderSource *source)
        {
            FILE *f = fopen(source->path, "rb");
            if (f == nullptr)
                return false;

            fseek(f, 0, SEEK_END);
            u64 len = ftell(f);

            fseek(f, 0, SEEK_SET);

            auto *buf = new unsigned char[len];

            if (fread(buf, len, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            fclose(f);

            bool res = shader_import_memory(result, buf, len);

            delete[] buf;

            return res;
        }

        bool shader_load_gpu(Shader *shader, const ShaderProgramSource *source)
        {
            u32 program = glCreateProgram();

            u32 vs = shader_compile(GL_VERTEX_SHADER, source->vert_src);
            if (vs == 0)
            {
                glDeleteShader(vs);
                glDeleteProgram(program);
                return false;
            }
            u32 fs = shader_compile(GL_FRAGMENT_SHADER, source->frag_src);
            if (fs == 0)
            {
                glDeleteShader(vs);
                glDeleteProgram(program);
                glDeleteShader(fs);
                glDeleteProgram(program);
                return false;
            }

            glAttachShader(program, vs);
            glAttachShader(program, fs);
            glLinkProgram(program);
            glValidateProgram(program);

            glDeleteShader(vs);
            glDeleteShader(fs);

            shader->id = program;
            return true;
        }

        bool shader_import(Shader *shader, const ShaderSource *source)
        {
            ShaderProgramSource shader_src;
            if (!shader_import_file(&shader_src, source))
                return false;

            return shader_load_gpu(shader, &shader_src);
        }

        u32 shader_compile(u32 type, const char *src)
        {
            u32 id = glCreateShader(type);
            glShaderSource(id, 1, &src, nullptr);
            glCompileShader(id);

            int res;
            glGetShaderiv(id, GL_COMPILE_STATUS, &res);

            if (res == GL_FALSE)
            {
                s32 len;
                glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);

                // char *message = str(len);
                // glGetShaderInfoLog(id, len, &len, message);

                // const char *shader_type = type == GL_VERTEX_SHADER ? "vertex" : "fragment";

                // fprintf(stderr, "Failed to compile %s shader: %s", shader_type, message);

                // free(message);

                glDeleteShader(id);
                return 0;
            }

            return id;
        }

    } // namespace ShaderImporter

} // namespace Vultr
