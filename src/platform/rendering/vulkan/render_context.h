#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <types/vector.h>
#include "device.h"
#include "swap_chain.h"
#include "vulkan_buffer.h"

namespace Vultr
{
	namespace Vulkan
	{
		struct Vertex
		{
			Vec2 position;
			Vec3 color;
		};
	} // namespace Vulkan
	namespace Platform
	{

		struct RenderContext
		{
			Vulkan::SwapChain swap_chain{};

			VkPipelineLayout pipeline_layout = nullptr;
			VkPipeline graphics_pipeline     = nullptr;
			//			VkDescriptorSetLayout descriptor_set_layout;

			Vector<VkCommandBuffer> command_buffers{};

			// TODO(Brandon): Don't do this here.
			const Vector<Vulkan::Vertex> vertices =
				Vector<Vulkan::Vertex>({{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}});
			const Vector<u16> indices = Vector<u16>({0, 1, 2, 2, 3, 0});
			Vulkan::VulkanBuffer vertex_buffer{};
			Vulkan::VulkanBuffer index_buffer{};
			f64 rotation = 0;
		};
	} // namespace Platform
} // namespace Vultr
