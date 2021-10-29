#include <render/types/index_buffer.h>
#include <glad/glad.h>

namespace Vultr
{
    IndexBuffer new_index_buffer(const void *indices, size_t count)
    {
        IndexBuffer ibo;

        glGenBuffers(1, &ibo);
        bind_index_buffer(ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * count, indices, GL_STATIC_DRAW);
        return ibo;
    }

    IndexBuffer new_index_buffer(u32 size)
    {
        IndexBuffer ibo;

        glGenBuffers(1, &ibo);
        bind_index_buffer(ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        return ibo;
    }

    void delete_index_buffer(IndexBuffer ibo)
    {
        glDeleteBuffers(1, &ibo);
    }

    void bind_index_buffer(IndexBuffer ibo)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    }

    void unbind_index_buffer()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

} // namespace Vultr
