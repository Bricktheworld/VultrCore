#include "vertex_buffer.h"
#include "upload_context.h"

namespace Vultr
{
	namespace Platform
	{
		VertexBuffer *init_vertex_buffer(UploadContext *c, void *data, size_t size)
		{
			auto *d             = Vulkan::get_device(c);

			auto staging_buffer = alloc_buffer(d, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

			fill_buffer(d, &staging_buffer, data, size);

			auto vertex_buffer = alloc_buffer(d, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
			copy_buffer(c, staging_buffer, vertex_buffer, size);

			free_buffer(d, &staging_buffer);

			auto *buffer          = v_alloc<VertexBuffer>();
			buffer->vertex_buffer = vertex_buffer;
			return buffer;
		}

		void destroy_vertex_buffer(Platform::RenderContext *c, VertexBuffer *buffer)
		{
			auto *d = Vulkan::get_device(c);
			free_buffer(d, &buffer->vertex_buffer);
			v_free(buffer);
		}
	} // namespace Platform
} // namespace Vultr
