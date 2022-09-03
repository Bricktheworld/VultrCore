#pragma once
#include <vulkan/vulkan.h>
#include "descriptor_set.h"
#include "texture.h"

namespace Vultr
{
	namespace Platform
	{
		struct ComputeShader
		{
			VkShaderModule module                   = nullptr;

			VkDescriptorSetLayout descriptor_layout = nullptr;

			Vector<VkDescriptorPool, Vulkan::MAX_FRAMES_IN_FLIGHT> descriptor_pools{};
			DescriptorSet descriptor_set{};
			Bitfield<64> updated             = MAX_BITFIELD<64>;

			VkPipelineLayout pipeline_layout = nullptr;
			VkPipeline pipeline              = nullptr;

			Platform::Mutex mutex{};
		};
	} // namespace Platform
} // namespace Vultr
