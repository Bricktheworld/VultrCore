#pragma once
#include <core/resource_allocator/resource_allocator.h>
#include "gpu_buffer.h"

namespace Vultr
{
#define MAX_DESCRIPTOR_SETS 128
	namespace Vulkan
	{
		VkDescriptorPool init_descriptor_pool(Device *d);
		void destroy_descriptor_pool(Device *d, VkDescriptorPool pool);
		size_t pad_size(Device *d, size_t size);
		struct UniformBufferBinding
		{
			Vulkan::GpuBuffer buffer{};
			void *mapped = nullptr;
		};
	} // namespace Vulkan

	namespace Platform
	{
		struct Texture;
		struct DescriptorSet
		{
			Vector<Vulkan::UniformBufferBinding> uniform_buffer_bindings{};
			Vector<Texture *> image_bindings{};

			Vector<VkDescriptorSet> vk_frame_descriptor_sets{};
			Bitfield<64> updated = MAX_BITFIELD<64>;
		};
	} // namespace Platform

} // namespace Vultr
