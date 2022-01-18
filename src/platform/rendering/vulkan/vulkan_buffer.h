#pragma once
#include "device.h"

namespace Vultr
{
	namespace Vulkan
	{
		struct VulkanBuffer
		{
			VkBuffer buffer       = nullptr;
			VkDeviceMemory memory = nullptr;
		};

		VulkanBuffer init_buffer(Device *d, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharing_mode);
		void copy_buffer(Device *d, VkBuffer dst_buffer, VkBuffer src_buffer, VkDeviceSize size);
		void destroy_buffer(Device *d, VulkanBuffer *buffer);
	} // namespace Vulkan
} // namespace Vultr