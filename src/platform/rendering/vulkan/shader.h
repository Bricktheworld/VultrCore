#pragma once
#include <vulkan/vulkan.h>
#include "device.h"
#include <filesystem/filesystem.h>
#include <platform/rendering.h>
#include "descriptor_set.h"

namespace Vultr
{
#define MAX_DESCRIPTOR_SETS 128
	namespace Platform
	{
		struct Shader
		{
			byte empty_bytes[8]{0};
			VkShaderModule vert_module = nullptr;
			VkShaderModule frag_module = nullptr;

			ShaderReflection reflection{};

			VkDescriptorSetLayout vk_custom_layout = nullptr;

			Vector<VkDescriptorPool, Vulkan::MAX_FRAMES_IN_FLIGHT> vk_descriptor_pools{};

			DescriptorSet descriptor_set_pool[MAX_DESCRIPTOR_SETS]{};
			Queue<DescriptorSet *, MAX_DESCRIPTOR_SETS> free_descriptor_sets{};
			Vector<DescriptorSet *, MAX_DESCRIPTOR_SETS> allocated_descriptor_sets{};
			Platform::Mutex mutex{};
		};
	} // namespace Platform
} // namespace Vultr
