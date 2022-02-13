#include "gpu_buffer.h"
#include "swap_chain.h"
#include "render_context.h"

namespace Vultr
{
	namespace Vulkan
	{
		static u32 find_memory_type(Device *d, u32 type_filter, VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties mem_properties;
			vkGetPhysicalDeviceMemoryProperties(d->physical_device, &mem_properties);

			for (u32 i = 0; i < mem_properties.memoryTypeCount; i++)
			{
				if ((type_filter & (1 << i)) && ((mem_properties.memoryTypes[i].propertyFlags & properties) == properties))
				{
					return i;
				}
			}
			THROW("Something went wrong trying to find a memory type!");
		}

		GpuBuffer alloc_buffer(Device *d, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, VkSharingMode sharing_mode)
		{
			GpuBuffer buffer_instance{.usage = memory_usage};
			VkBufferCreateInfo buffer_info{
				.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size        = size,
				.usage       = usage,
				.sharingMode = sharing_mode,
			};

			VmaAllocationCreateInfo alloc_info{memory_usage};

			VK_CHECK(vmaCreateBuffer(d->allocator, &buffer_info, &alloc_info, &buffer_instance.buffer, &buffer_instance.memory, nullptr));
			return buffer_instance;
		}

		void fill_buffer(Device *d, GpuBuffer *buffer, void *data, size_t size)
		{
			ASSERT(buffer->usage != VMA_MEMORY_USAGE_GPU_TO_CPU && buffer->usage != VMA_MEMORY_USAGE_GPU_TO_CPU, "Cannot fill buffer that is on the GPU!");
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
			vkEndCommandBuffer(cmd);

			end_cmd_buffer(&c->cmd_pool);

			VkSubmitInfo submit_info{
				.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.commandBufferCount = 1,
				.pCommandBuffers    = &cmd,
			};

			vkQueueSubmit(d->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);

			// TODO(Brandon): Maybe don't wait here, and instead have a fence.
			vkQueueWaitIdle(d->graphics_queue);
		}

		void free_buffer(Device *d, GpuBuffer *buffer)
		{
			vmaDestroyBuffer(d->allocator, buffer->buffer, buffer->memory);
			buffer->buffer = nullptr;
			buffer->memory = nullptr;
		}
	} // namespace Vulkan
} // namespace Vultr
