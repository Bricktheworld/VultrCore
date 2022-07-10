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

			VkPipelineLayout pipeline_layout = nullptr;
			VkPipeline pipeline              = nullptr;

			Platform::Mutex mutex{};

			Texture *input  = nullptr;
			Texture *output = nullptr;
			Vector<VkDescriptorSet> frame_descriptor_sets{};
			Bitfield<64> updated = MAX_BITFIELD<64>;
		};
	} // namespace Platform
} // namespace Vultr
