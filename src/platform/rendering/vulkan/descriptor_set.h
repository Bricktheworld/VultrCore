#pragma once
#include "gpu_buffer.h"

namespace Vultr
{
	namespace Vulkan
	{
		struct DescriptorSet
		{
			struct MappedBuffer
			{
				Vulkan::GpuBuffer buffer{};
				void *mapped = nullptr;
			};
			const Platform::DescriptorLayout *layout = nullptr;
			VkDescriptorSet vk_set                   = nullptr;
			Hashmap<u32, MappedBuffer> binding_buffers{};
			Hashmap<u32, Option<Platform::Texture>> binding_samplers{};
			bool updated = false;
		};

		VkDescriptorPool init_descriptor_pool(Device *d);
		void destroy_descriptor_pool(Device *d, VkDescriptorPool pool);

		DescriptorSet init_descriptor_set(Device *d, VkDescriptorPool pool, Platform::DescriptorLayout *layout);
		void destroy_descriptor_set(Device *d, DescriptorSet *buffer);
	} // namespace Vulkan

	namespace Platform
	{
		struct DescriptorLayout
		{
			Vector<DescriptorSetBinding> bindings{};
			u32 max_objects                 = 0;
			VkDescriptorSetLayout vk_layout = nullptr;
		};
	} // namespace Platform

} // namespace Vultr