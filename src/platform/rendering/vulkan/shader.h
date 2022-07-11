#pragma once
#include <vulkan/vulkan.h>
#include "device.h"
#include <filesystem/filesystem.h>
#include <platform/rendering.h>
#include "descriptor_set.h"
#include "pipeline.h"

namespace Vultr
{
	namespace Platform
	{
		struct Shader
		{
			VkShaderModule vert_module = nullptr;
			VkShaderModule frag_module = nullptr;

			ShaderReflection reflection{};

			VkDescriptorSetLayout vk_custom_layout = nullptr;

			Vector<VkDescriptorPool, Vulkan::MAX_FRAMES_IN_FLIGHT> vk_descriptor_pools{};

			DescriptorSet descriptor_set_pool[MAX_DESCRIPTOR_SETS]{};
			Queue<DescriptorSet *, MAX_DESCRIPTOR_SETS> free_descriptor_sets{};
			Vector<DescriptorSet *, MAX_DESCRIPTOR_SETS> allocated_descriptor_sets{};
			Platform::Mutex mutex{};

			HashTable<Framebuffer *> framebuffers_with_pipelines{};
		};
	} // namespace Platform

	namespace Vulkan
	{
		GraphicsPipeline *get_or_create_pipeline(Platform::RenderContext *c, Platform::Shader *shader, const Platform::GraphicsPipelineInfo &info, Option<Platform::Framebuffer *> framebuffer = None);
	}
} // namespace Vultr
