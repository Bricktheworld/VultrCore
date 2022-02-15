#include "index_buffer.h"
#include "render_context.h"
#include <vulkan/vulkan.h>

namespace Vultr
{
	namespace Platform
	{
		IndexBuffer *init_index_buffer(UploadContext *c, u16 *data, size_t size)
		{
			auto *d             = Vulkan::get_device(c);

			auto staging_buffer = alloc_buffer(d, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

			fill_buffer(d, &staging_buffer, data, size);

			auto index_buffer = alloc_buffer(d, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
			copy_buffer(c, index_buffer, staging_buffer, size);

			free_buffer(d, &staging_buffer);

			auto *buffer         = v_alloc<IndexBuffer>();
			buffer->index_buffer = index_buffer;
			return buffer;
		}
		void destroy_index_buffer(UploadContext *c, IndexBuffer *buffer)
		{
			auto *d = Vulkan::get_device(c);
			free_buffer(d, &buffer->index_buffer);
			v_free(buffer);
		}

		void bind_index_buffer(CmdBuffer *cmd, IndexBuffer *ibo)
		{
			ASSERT(ibo != nullptr && ibo->index_buffer.buffer != nullptr, "Cannot bind nullptr index buffer!");
			vkCmdBindIndexBuffer(cmd->cmd_buffer, ibo->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);
		}
	} // namespace Platform
} // namespace Vultr
