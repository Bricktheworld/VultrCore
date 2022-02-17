#pragma once
#include "gpu_buffer.h"

namespace Vultr
{
	namespace Vulkan
	{
		struct DescriptorSetBuffer
		{
			const Platform::DescriptorLayout *layout = nullptr;
			VkDescriptorSet vk_set                   = nullptr;
			Vulkan::GpuBuffer buffer{};
			void *mapped = nullptr;
			bool updated = false;
		};

		VkDescriptorPool init_descriptor_pool(Device *d);
		void destroy_descriptor_pool(Device *d, VkDescriptorPool pool);

		DescriptorSetBuffer init_descriptor_set_buffer(Device *d, VkDescriptorPool pool, Platform::DescriptorLayout *layout);
		void destroy_descriptor_set_buffer(Device *d, DescriptorSetBuffer *buffer);
	} // namespace Vulkan

	namespace Platform
	{
		struct DescriptorLayout
		{
			size_t size                     = 0;
			u32 max_objects                 = 0;
			VkDescriptorSetLayout vk_layout = nullptr;
		};
	} // namespace Platform

} // namespace Vultr