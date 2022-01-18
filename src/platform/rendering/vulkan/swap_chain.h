#pragma once
#include "device.h"

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

			u32 current_frame = 0;

			Device device;
			VkCommandPool graphics_command_pool = nullptr;

			bool framebuffer_was_resized        = false;
		};

		SwapChain init_swapchain(const Device &device, const Platform::Window *window);
		void recreate_swapchain(SwapChain *sc, const Platform::Window *window);
		ErrorOr<u32> acquire_swapchain(SwapChain *sc);
		ErrorOr<void> submit_swapchain(SwapChain *sc, u32 image_index, u32 command_buffer_count, VkCommandBuffer *command_buffers);
		void destroy_swapchain(SwapChain *sc);
	}; // namespace Vulkan
} // namespace Vultr