#include "vulkan_buffer.h"
#include "swap_chain.h"

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

		VulkanBuffer alloc_buffer(Device *d, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharing_mode)
		{
			VulkanBuffer buffer_instance{};
			VkBufferCreateInfo buffer_info{
				.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size        = size,
				.usage       = usage,
				.sharingMode = sharing_mode,
			};

			PRODUCTION_ASSERT(vkCreateBuffer(d->device, &buffer_info, nullptr, &buffer_instance.buffer) == VK_SUCCESS, "Failed to create vulkan buffer!");

			VkMemoryRequirements mem_requirements;
			vkGetBufferMemoryRequirements(d->device, buffer_instance.buffer, &mem_requirements);

			VkMemoryAllocateInfo alloc_info{
				.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize  = mem_requirements.size,
				.memoryTypeIndex = find_memory_type(d, mem_requirements.memoryTypeBits, properties),
			};

			PRODUCTION_ASSERT(vkAllocateMemory(d->device, &alloc_info, nullptr, &buffer_instance.memory) == VK_SUCCESS, "Failed to allocate memory for vertex buffer!");
			vkBindBufferMemory(d->device, buffer_instance.buffer, buffer_instance.memory, 0);
			return buffer_instance;
		}

		void copy_buffer(SwapChain *sc, VulkanBuffer dst_buffer, VulkanBuffer src_buffer, VkDeviceSize size)
		{
			auto *d = &sc->device;
			VkCommandBufferAllocateInfo alloc_info{
				.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool        = sc->graphics_command_pools[0],
				.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};

			VkCommandBuffer command_buffer;
			vkAllocateCommandBuffers(d->device, &alloc_info, &command_buffer);

			VkCommandBufferBeginInfo begin_info{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			};
			vkBeginCommandBuffer(command_buffer, &begin_info);

			VkBufferCopy copy_region{.size = size};
			vkCmdCopyBuffer(command_buffer, src_buffer.buffer, dst_buffer.buffer, 1, &copy_region);
			vkEndCommandBuffer(command_buffer);

			VkSubmitInfo submit_info{
				.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.commandBufferCount = 1,
				.pCommandBuffers    = &command_buffer,
			};

			vkQueueSubmit(d->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
			vkQueueWaitIdle(d->graphics_queue);
			vkFreeCommandBuffers(d->device, sc->graphics_command_pools[0], 1, &command_buffer);
		}

		void free_buffer(Device *d, VulkanBuffer *buffer)
		{
			vkDestroyBuffer(d->device, buffer->buffer, nullptr);
			vkFreeMemory(d->device, buffer->memory, nullptr);
			buffer->buffer = nullptr;
			buffer->memory = nullptr;
		}
	} // namespace Vulkan
} // namespace Vultr