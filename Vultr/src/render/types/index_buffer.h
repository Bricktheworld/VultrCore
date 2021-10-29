#pragma once
#include <types/types.h>

namespace Vultr
{
    typedef u32 IndexBuffer;

    IndexBuffer new_index_buffer(const void *indices, size_t count);
    IndexBuffer new_index_buffer(u32 size);

    void delete_index_buffer(IndexBuffer ibo);

    void bind_index_buffer(IndexBuffer ibo);
    void unbind_index_buffer();
} // namespace Vultr
