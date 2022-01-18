#pragma once
#include "swap_chain.h"

namespace Vultr
{
	namespace Vulkan
	{
		struct VulkanBuffer
		{
			VkBuffer buffer       = nullptr;
			VkDeviceMemory memory = nullptr;
		};

		VulkanBuffer alloc_buffer(Device *d, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharing_mode);
		void copy_buffer(SwapChain *sc, VulkanBuffer dst_buffer, VulkanBuffer src_buffer, VkDeviceSize size);
		void free_buffer(Device *d, VulkanBuffer *buffer);
	} // namespace Vulkan
} // namespace Vultr