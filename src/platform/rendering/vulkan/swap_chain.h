#pragma once
#include "device.h"
#include "gpu_buffer.h"
#include "command_pool.h"
#include <types/tuple.h>

namespace Vultr
{
	namespace Vulkan
	{
		struct SwapChain
		{
			VkSwapchainKHR swap_chain = nullptr;
			VkFormat image_format{};
			VkExtent2D extent{};
			Vector<VkImage> images{};
			Vector<VkImageView> image_views{};
			Vector<VkFramebuffer> framebuffers{};
			VkRenderPass render_pass = nullptr;

			VkSemaphore image_available_semaphores[Vulkan::MAX_FRAMES_IN_FLIGHT]{};
			VkSemaphore render_finished_semaphores[Vulkan::MAX_FRAMES_IN_FLIGHT]{};
			VkFence in_flight_fences[Vulkan::MAX_FRAMES_IN_FLIGHT]{};
			Vector<VkFence> images_in_flight{};
			//
			//			Vector<VulkanBuffer> uniform_buffers{};
			//			VkDescriptorPool descriptor_pool = nullptr;
			u32 current_frame = 0;

			Device device{};
			Vector<CommandPool> render_command_pools{};

			bool framebuffer_was_resized = false;
		};

		SwapChain init_swapchain(const Device &device, const Platform::Window *window);
		void recreate_swapchain(SwapChain *sc, const Platform::Window *window);
		ErrorOr<Tuple<u32, CommandPool *, VkFramebuffer>> acquire_swapchain(SwapChain *sc);
		ErrorOr<void> submit_swapchain(SwapChain *sc, u32 image_index, u32 command_buffer_count, VkCommandBuffer *command_buffers);
		ErrorOr<void> queue_cmd_buffer(SwapChain *sc, VkCommandBuffer command_buffer, VkFence fence = VK_NULL_HANDLE);
		ErrorOr<void> wait_queue_cmd_buffer(SwapChain *sc, VkCommandBuffer command_buffer);
		void destroy_swapchain(SwapChain *sc);
	}; // namespace Vulkan
} // namespace Vultr
