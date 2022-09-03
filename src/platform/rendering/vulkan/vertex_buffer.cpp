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
			copy_buffer(c, vertex_buffer, staging_buffer, size);

			unsafe_free_buffer(d, &staging_buffer);

			auto *buffer          = v_alloc<VertexBuffer>();
			buffer->vertex_buffer = vertex_buffer;

			return buffer;
		}

		void destroy_vertex_buffer(RenderContext *c, VertexBuffer *buffer)
		{
			auto *sc = Vulkan::get_swapchain(c);
			free_buffer(sc, &buffer->vertex_buffer);
			v_free(buffer);
		}

		void bind_vertex_buffer(CmdBuffer *cmd, VertexBuffer *vbo)
		{
			ASSERT(vbo != nullptr && vbo->vertex_buffer.buffer != nullptr, "Cannot bind nullptr vertex buffer!");
			VkBuffer vertex_buffers[] = {vbo->vertex_buffer.buffer};
			VkDeviceSize offsets[]    = {0};
			vkCmdBindVertexBuffers(cmd->cmd_buffer, 0, 1, vertex_buffers, offsets);
			Vulkan::depend_resource(cmd, &vbo->vertex_buffer);
		}
	} // namespace Platform
} // namespace Vultr
