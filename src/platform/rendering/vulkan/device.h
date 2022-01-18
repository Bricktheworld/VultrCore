#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "constants.h"
#include <types/vector.h>
#include "../../platform_impl.h"

namespace Vultr
{
	namespace Vulkan
	{
		struct Device
		{
			VkInstance instance              = nullptr;
			VkPhysicalDevice physical_device = nullptr;

			VkDevice device                  = nullptr;
			VkSurfaceKHR surface             = nullptr;
			VkSwapchainKHR swap_chain        = nullptr;
			VkFormat swap_chain_image_format{};
			VkExtent2D swap_chain_extent{};
			Vector<VkImage> swap_chain_images{};
			Vector<VkImageView> swap_chain_image_views{};

			VkQueue graphics_queue                   = nullptr;
			VkCommandPool graphics_command_pool      = nullptr;
			VkQueue present_queue                    = nullptr;

			VkDebugUtilsMessengerEXT debug_messenger = nullptr;
		};

		Device init_device(const Platform::Window *window, bool debug, PFN_vkDebugUtilsMessengerCallbackEXT debug_cb);
		void recreate_swapchain(Device *d, const Platform::Window *window);
		void destroy_device(Device *d);

		void wait_idle(Device *d);

	} // namespace Vulkan
} // namespace Vultr