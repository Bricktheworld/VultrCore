#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "constants.h"
#include <types/vector.h>
#include "../../platform_impl.h"
#include <vma/vk_mem_alloc.h>

namespace Vultr
{
	namespace Vulkan
	{
		struct Device
		{
			VkInstance instance              = nullptr;
			VkPhysicalDevice physical_device = nullptr;
			VkPhysicalDeviceProperties properties{};

			VkDevice device                          = nullptr;
			VkSurfaceKHR surface                     = nullptr;

			VkQueue graphics_queue                   = nullptr;
			VkQueue present_queue                    = nullptr;

			VkDebugUtilsMessengerEXT debug_messenger = nullptr;

			VmaAllocator allocator                   = nullptr;
		};

		Device init_device(const Platform::Window *window, bool debug, PFN_vkDebugUtilsMessengerCallbackEXT debug_cb);
		void destroy_device(Device *d);

		// void map_memory(Device *d, void *)

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
		size_t min_ubo_alignment(Device *d);

		void wait_idle(Device *d);

	} // namespace Vulkan
} // namespace Vultr
