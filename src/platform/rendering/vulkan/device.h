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
			VkInstance instance                      = nullptr;
			VkPhysicalDevice physical_device         = nullptr;

			VkDevice device                          = nullptr;
			VkSurfaceKHR surface                     = nullptr;

			VkQueue graphics_queue                   = nullptr;
			VkQueue present_queue                    = nullptr;

			VkDebugUtilsMessengerEXT debug_messenger = nullptr;
		};

		Device init_device(const Platform::Window *window, bool debug, PFN_vkDebugUtilsMessengerCallbackEXT debug_cb);
		void destroy_device(Device *d);

		struct QueueFamilyIndices
		{
			Option<u32> graphics_family = None;
			Option<u32> present_family  = None;
		};

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			Vector<VkSurfaceFormatKHR> formats;
			Vector<VkPresentModeKHR> present_modes;
		};

		QueueFamilyIndices find_queue_families(Device *d);
		SwapChainSupportDetails query_swap_chain_support(Device *d);

		void wait_idle(Device *d);

	} // namespace Vulkan
} // namespace Vultr