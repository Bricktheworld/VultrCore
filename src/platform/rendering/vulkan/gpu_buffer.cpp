#include "gpu_buffer.h"
#include "swap_chain.h"
#include "render_context.h"

namespace Vultr
{
	namespace Vulkan
	{
		GpuBuffer alloc_buffer(Device *d, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, VkMemoryPropertyFlags flags, VkSharingMode sharing_mode)
		{
			GpuBuffer buffer_instance{.usage = memory_usage};
			VkBufferCreateInfo buffer_info{
				.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size        = size,
				.usage       = usage,
				.sharingMode = sharing_mode,
			};

			VmaAllocationCreateInfo alloc_info{
				.usage         = memory_usage,
				.requiredFlags = flags,
			};

			VK_CHECK(vmaCreateBuffer(d->allocator, &buffer_info, &alloc_info, &buffer_instance.buffer, &buffer_instance.memory, nullptr));
			return buffer_instance;
		}

		void fill_buffer(Device *d, GpuBuffer *buffer, void *data, size_t size)
		{
			ASSERT(buffer->usage != VMA_MEMORY_USAGE_GPU_TO_CPU && buffer->usage != VMA_MEMORY_USAGE_GPU_ONLY, "Cannot fill buffer that is on the GPU!");
			void *mapped = nullptr;
			vmaMapMemory(d->allocator, buffer->memory, &mapped);
			memcpy(mapped, data, size);
			vmaUnmapMemory(d->allocator, buffer->memory);
		}

		void copy_buffer(Platform::UploadContext *c, GpuBuffer dst_buffer, GpuBuffer src_buffer, VkDeviceSize size)
		{
			auto *d  = get_device(c);

			auto cmd = begin_cmd_buffer(d, &c->cmd_pool);

			VkBufferCopy copy_region{.size = size};
			vkCmdCopyBuffer(cmd, src_buffer.buffer, dst_buffer.buffer, 1, &copy_region);

			end_cmd_buffer(cmd, &c->cmd_pool);

			VkSubmitInfo submit_info{
				.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.commandBufferCount = 1,
				.pCommandBuffers    = &cmd,
			};

			vkResetFences(d->device, 1, &c->cmd_pool.fence);
			vkQueueSubmit(d->graphics_queue, 1, &submit_info, c->cmd_pool.fence);

			// TODO(Brandon): Maybe don't wait here, and instead have a fence.
			vkWaitForFences(d->device, 1, &c->cmd_pool.fence, VK_TRUE, UINT64_MAX);
		}

		void *map_buffer(Device *d, GpuBuffer *buffer)
		{
			ASSERT(buffer->usage != VMA_MEMORY_USAGE_GPU_TO_CPU && buffer->usage != VMA_MEMORY_USAGE_GPU_ONLY, "Cannot map memory that is not visible to the CPU!");
			void *mapped = nullptr;
			VK_CHECK(vmaMapMemory(d->allocator, buffer->memory, &mapped));
			ASSERT(mapped != nullptr, "Failed to map buffer!");
			return mapped;
		}
		void unmap_buffer(Device *d, GpuBuffer *buffer) { vmaUnmapMemory(d->allocator, buffer->memory); }

		void free_buffer(Device *d, GpuBuffer *buffer)
		{
			vmaDestroyBuffer(d->allocator, buffer->buffer, buffer->memory);
			buffer->buffer = nullptr;
			buffer->memory = nullptr;
		}
	} // namespace Vulkan
} // namespace Vultr
