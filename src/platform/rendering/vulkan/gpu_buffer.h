#pragma once
#include <vulkan/vulkan.h>
#include "upload_context.h"

namespace Vultr
{
	namespace Vulkan
	{
		struct GpuBuffer
		{
			VkBuffer buffer      = nullptr;
			VmaAllocation memory = nullptr;
			VmaMemoryUsage usage = VMA_MEMORY_USAGE_GPU_ONLY;
		};

		GpuBuffer alloc_buffer(Device *d, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY, VkMemoryPropertyFlags flags = 0,
							   VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE);
		void fill_buffer(Device *d, GpuBuffer *buffer, void *data, size_t size);
		void copy_buffer(Platform::UploadContext *c, GpuBuffer dst_buffer, GpuBuffer src_buffer, VkDeviceSize size);
		void *map_buffer(Device *d, GpuBuffer *buffer);
		void unmap_buffer(Device *d, GpuBuffer *buffer);
		void free_buffer(Device *d, GpuBuffer *buffer);
	} // namespace Vulkan
} // namespace Vultr
