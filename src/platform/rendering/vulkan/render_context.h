#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <types/vector.h>
#include "device.h"

namespace Vultr
{
	namespace Platform
	{
		struct RenderContext
		{
			Vulkan::Device device{};

			VkRenderPass render_pass         = nullptr;
			VkPipelineLayout pipeline_layout = nullptr;
			VkPipeline graphics_pipeline     = nullptr;

			Vector<VkFramebuffer> swap_chain_framebuffers{};
			Vector<VkCommandBuffer> command_buffers{};

			VkSemaphore image_available_semaphores[Vulkan::MAX_FRAMES_IN_FLIGHT]{};
			VkSemaphore render_finished_semaphores[Vulkan::MAX_FRAMES_IN_FLIGHT]{};
			VkFence in_flight_fences[Vulkan::MAX_FRAMES_IN_FLIGHT]{};
			Vector<VkFence> images_in_flight{};
			u32 current_frame            = 0;

			bool framebuffer_was_resized = false;

			//			const Vector<Vertex> vertices       = Vector<Vertex>({{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, {{-0.5f, 0.5f},
			//{1.0f, 1.0f, 1.0f}}}); 			const Vector<u16> indices           = Vector<u16>({0, 1, 2, 2, 3, 0}); 			VkBuffer vertex_buffer              = nullptr; 			VkDeviceMemory
			// vertex_buffer_memory = nullptr; 			VkBuffer index_buffer               = nullptr; 			VkDeviceMemory index_buffer_memory  = nullptr;
		};
	} // namespace Platform
} // namespace Vultr
