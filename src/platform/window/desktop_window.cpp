#include <glad/glad.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "types/vector.h"
#include "types/optional.h"
#include "math/clamp.h"
#include <core/vultr_core.h>

namespace Vultr::Platform
{
	static constexpr const char *VALIDATION_LAYERS[] = {"VK_LAYER_KHRONOS_validation"};
	static constexpr u32 VALIDATION_LAYERS_COUNT     = sizeof(VALIDATION_LAYERS) / sizeof(const char *);
	static constexpr const char *DEVICE_EXTENSIONS[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	static constexpr u32 DEVICE_EXTENSION_COUNT      = sizeof(DEVICE_EXTENSIONS) / sizeof(const char *);

	struct Window
	{
		GLFWwindow *glfw                    = nullptr;

		VkInstance vk_instance              = nullptr;
		VkPhysicalDevice vk_physical_device = nullptr;
		VkDevice vk_logical_device          = nullptr;
		VkSurfaceKHR vk_surface             = nullptr;
		VkSwapchainKHR swap_chain           = nullptr;

		VkQueue vk_graphics_queue           = nullptr;
		VkQueue vk_present_queue            = nullptr;

		Vector<VkImage> swap_chain_images;
		Vector<VkImageView> swap_chain_image_views;

		// TODO(Brandon): Don't hard-code validation.
		bool debug                                  = true;
		VkDebugUtilsMessengerEXT vk_debug_messenger = nullptr;
	};

	struct VkQueueFamilyIndices
	{
		Option<u32> graphics_family = None;
		Option<u32> present_family  = None;
	};

	struct VkSwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		Vector<VkSurfaceFormatKHR> formats;
		Vector<VkPresentModeKHR> present_modes;
	};

	struct Monitor
	{
		GLFWmonitor *glfw = nullptr;
	};

	static GLFWmonitor *get_monitor_or_primary(Monitor *monitor)
	{
		if (monitor != nullptr)
		{
			ASSERT(monitor->glfw != nullptr, "Invalid monitor");
			return monitor->glfw;
		}
		else
		{
			auto *res = glfwGetPrimaryMonitor();
			ASSERT(res != nullptr, "Failed to get primary monitor.");
			return res;
		}
	}

	static bool vk_has_validation(VkLayerProperties available_layers[], size_t available_layers_count)
	{
		for (size_t i = 0; i < VALIDATION_LAYERS_COUNT; i++)
		{
			bool layer_found = false;

			for (size_t j = 0; j < available_layers_count; j++)
			{
				if (strcmp(VALIDATION_LAYERS[i], available_layers[j].layerName) == 0)
				{
					layer_found = true;
					break;
				}
			}
			if (!layer_found)
				return false;
		}
		return true;
	}

	// TODO(Brandon): Move this outta here.
	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_cb(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
												   void *user_data)
	{
		if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			fprintf(stderr, "(Vulkan): %s\n", callback_data->pMessage);
		}
		else
		{
			fprintf(stdout, "(Vulkan): %s\n", callback_data->pMessage);
		}

		return VK_FALSE;
	}

	static void vk_init_debug_messenger(Window *window)
	{
		VkDebugUtilsMessengerCreateInfoEXT create_info{
			.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = debug_cb,
			.pUserData       = nullptr,
		};

		auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(window->vk_instance, "vkCreateDebugUtilsMessengerEXT");
		PRODUCTION_ASSERT(vkCreateDebugUtilsMessengerEXT != nullptr, "Failed to load vkCreateDebugUtilsMessengerEXT!");
		PRODUCTION_ASSERT(vkCreateDebugUtilsMessengerEXT(window->vk_instance, &create_info, nullptr, &window->vk_debug_messenger) == VK_SUCCESS, "Failed to create vulkan debug messenger!");
	}

	static void vk_destroy_debug_messenger(Window *window)
	{
		auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(window->vk_instance, "vkDestroyDebugUtilsMessengerEXT");
		PRODUCTION_ASSERT(vkDestroyDebugUtilsMessengerEXT != nullptr, "Failed to load vkDestroyDebugUtilsMessengerEXT!");
		vkDestroyDebugUtilsMessengerEXT(window->vk_instance, window->vk_debug_messenger, nullptr);
	}

	static bool vk_is_complete(const VkQueueFamilyIndices &indices) { return indices.graphics_family && indices.present_family; }
	static bool vk_is_complete(const VkSwapChainSupportDetails &details) { return !details.present_modes.empty() && !details.formats.empty(); }

	static VkQueueFamilyIndices vk_find_queue_families(VkPhysicalDevice vk_physical_device, Window *window)
	{
		VkQueueFamilyIndices indices{};

		u32 count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &count, nullptr);

		VkQueueFamilyProperties properties[count];
		vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &count, properties);

		for (u32 i = 0; i < count; i++)
		{
			if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphics_family = i;
			}

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, i, window->vk_surface, &present_support);
			if (present_support)
			{
				indices.present_family = i;
			}

			if (vk_is_complete(indices))
				break;
		}

		return indices;
	}

	static VkSwapChainSupportDetails vk_query_swap_chain_support(VkPhysicalDevice device, Window *window)
	{
		VkSwapChainSupportDetails details{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, window->vk_surface, &details.capabilities);

		u32 format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->vk_surface, &format_count, nullptr);

		if (format_count != 0)
		{
			details.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->vk_surface, &format_count, &details.formats[0]);
		}

		u32 present_modes_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, window->vk_surface, &present_modes_count, nullptr);

		if (present_modes_count != 0)
		{
			details.present_modes.resize(present_modes_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, window->vk_surface, &present_modes_count, &details.present_modes[0]);
		}

		return details;
	}

	static bool vk_check_device_extension_support(VkPhysicalDevice device)
	{
		u32 count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

		VkExtensionProperties extensions[count];
		vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions);

		for (const auto *required_extension : DEVICE_EXTENSIONS)
		{
			bool found = false;
			for (const auto &extension : extensions)
			{
				if (strcmp(required_extension, extension.extensionName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
				return false;
		}

		return true;
	}

	static u32 vk_rate_device_suitability(VkPhysicalDevice device, Window *window)
	{
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);

		u32 score    = 0;

		auto indices = vk_find_queue_families(device, window);
		if (!vk_is_complete(indices))
			return 0;

		if (!vk_check_device_extension_support(device))
			return 0;

		auto details = vk_query_swap_chain_support(device, window);
		if (!vk_is_complete(details))
			return 0;

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		score += properties.limits.maxImageDimension2D;
		return score;
	}

	static void vk_pick_physical_device(Window *window)
	{
		u32 device_count = 0;
		vkEnumeratePhysicalDevices(window->vk_instance, &device_count, nullptr);

		PRODUCTION_ASSERT(device_count != 0, "No GPUs on this machine support Vulkan!");

		VkPhysicalDevice devices[device_count];
		vkEnumeratePhysicalDevices(window->vk_instance, &device_count, devices);

		u32 best_score               = 0;
		VkPhysicalDevice best_device = nullptr;
		for (auto device : devices)
		{
			u32 score = vk_rate_device_suitability(device, window);
			if (score > best_score)
			{
				best_score  = score;
				best_device = device;
			}
		}

		PRODUCTION_ASSERT(best_score != 0 && best_device != nullptr, "No GPUs on this machine are supported!");
		window->vk_physical_device = best_device;
	}

	static VkSurfaceFormatKHR vk_pick_swap_chain_surface_format(const Vector<VkSurfaceFormatKHR> &available_formats)
	{
		for (const auto &format : available_formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		return available_formats[0];
	}

	static VkPresentModeKHR vk_pick_present_mode(const Vector<VkPresentModeKHR> &available_modes)
	{
		for (const auto &mode : available_modes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return mode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	static VkExtent2D vk_pick_swap_extent(Window *window, const VkSurfaceCapabilitiesKHR &capabilities)
	{
		if (capabilities.currentExtent.width != U32Max)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window->glfw, &width, &height);

			VkExtent2D actualExtent = {static_cast<u32>(width), static_cast<u32>(height)};

			actualExtent.width      = Vultr::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height     = Vultr::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	static void vk_init_swap_chain(Window *window)
	{
		auto details        = vk_query_swap_chain_support(window->vk_physical_device, window);

		auto surface_format = vk_pick_swap_chain_surface_format(details.formats);
		auto present_mode   = vk_pick_present_mode(details.present_modes);
		auto extent         = vk_pick_swap_extent(window, details.capabilities);

		u32 image_count     = details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount)
		{
			image_count = details.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR create_info{
			.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface          = window->vk_surface,
			.minImageCount    = image_count,
			.imageFormat      = surface_format.format,
			.imageColorSpace  = surface_format.colorSpace,
			.imageExtent      = extent,
			.imageArrayLayers = 1,
			.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		};

		auto indices               = vk_find_queue_families(window->vk_physical_device, window);
		u32 queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

		if (indices.graphics_family != indices.present_family)
		{
			create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices   = queue_family_indices;
		}
		else
		{
			create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices   = nullptr;
		}

		create_info.preTransform   = details.capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode    = present_mode;
		create_info.clipped        = VK_TRUE;
		create_info.oldSwapchain   = VK_NULL_HANDLE;
		PRODUCTION_ASSERT(vkCreateSwapchainKHR(window->vk_logical_device, &create_info, nullptr, &window->swap_chain) == VK_SUCCESS, "Failed to create Vulkan swap chain!");
		vkGetSwapchainImagesKHR(window->vk_logical_device, window->swap_chain, &image_count, nullptr);
		window->swap_chain_images.resize(image_count);
		vkGetSwapchainImagesKHR(window->vk_logical_device, window->swap_chain, &image_count, &window->swap_chain_images[0]);
	}

	static void vk_init_image_views(Window *window)
	{
		window->swap_chain_image_views.resize(window->swap_chain_images.size());

		for(size_t i = 0; i < window->swap_chain_image_views.size(); i++)
		{
			VkImageViewCreateInfo create_info{
				.sType =VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = window->swap_chain_images[i],

			};
		}
	}

	static void vk_init_logical_device(Window *window)
	{
		auto indices       = vk_find_queue_families(window->vk_physical_device, window);

		f32 queue_priority = 1.0f;

		Vector<u32, 2> queue_indices;
		queue_indices.push_if_not_contains(indices.graphics_family.value());
		queue_indices.push_if_not_contains(indices.present_family.value());

		VkDeviceQueueCreateInfo queue_create_infos[queue_indices.size()];

		for (int i = 0; i < queue_indices.size(); i++)
		{
			queue_create_infos[i] = {
				.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queue_indices[i],
				.queueCount       = 1,
				.pQueuePriorities = &queue_priority,
			};
		}
		VkPhysicalDeviceFeatures device_features{};

		VkDeviceCreateInfo create_info{
			.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount    = (u32)queue_indices.size(),
			.pQueueCreateInfos       = queue_create_infos,
			.enabledLayerCount       = VALIDATION_LAYERS_COUNT,
			.ppEnabledLayerNames     = VALIDATION_LAYERS,
			.enabledExtensionCount   = DEVICE_EXTENSION_COUNT,
			.ppEnabledExtensionNames = DEVICE_EXTENSIONS,
			.pEnabledFeatures        = &device_features,
		};
		PRODUCTION_ASSERT(vkCreateDevice(window->vk_physical_device, &create_info, nullptr, &window->vk_logical_device) == VK_SUCCESS, "Failed to create vulkan logical device!");
		vkGetDeviceQueue(window->vk_logical_device, indices.graphics_family.value(), 0, &window->vk_graphics_queue);
		vkGetDeviceQueue(window->vk_logical_device, indices.present_family.value(), 0, &window->vk_present_queue);
	}

	static void vk_init_surface(Window *window)
	{
		PRODUCTION_ASSERT(glfwCreateWindowSurface(window->vk_instance, window->glfw, nullptr, &window->vk_surface) == VK_SUCCESS, "Failed to create vulkan window surface!");
	}

	static void vk_init(Window *window, const char *title)
	{
		VkApplicationInfo vk_application_info{
			.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName   = title,
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName        = "No Engine",
			.engineVersion      = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion         = VK_API_VERSION_1_0,
		};

		VkInstanceCreateInfo vk_instance_create_info{
			.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &vk_application_info,
		};

		u32 glfw_vk_extension_count     = 0;
		const char **glfw_vk_extensions = glfwGetRequiredInstanceExtensions(&glfw_vk_extension_count);
		Vector vk_extensions(glfw_vk_extensions, glfw_vk_extension_count);

		if (window->debug)
		{
			vk_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		vk_instance_create_info.enabledExtensionCount   = vk_extensions.size();
		vk_instance_create_info.ppEnabledExtensionNames = &vk_extensions[0];

		if (window->debug)
		{
			u32 layer_count;
			vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

			VkLayerProperties available_validation_layers[layer_count];
			vkEnumerateInstanceLayerProperties(&layer_count, available_validation_layers);

			PRODUCTION_ASSERT(vk_has_validation(available_validation_layers, layer_count), "Vulkan failed to find validation layers!");

			vk_instance_create_info.enabledLayerCount   = 1;
			vk_instance_create_info.ppEnabledLayerNames = VALIDATION_LAYERS;
		}
		else
		{
			vk_instance_create_info.enabledLayerCount = 0;
		}

		PRODUCTION_ASSERT(vkCreateInstance(&vk_instance_create_info, nullptr, &window->vk_instance) == VK_SUCCESS, "Failed to init vulkan.");
		vk_init_debug_messenger(window);
		vk_init_surface(window);
		vk_pick_physical_device(window);
		vk_init_logical_device(window);
		vk_init_swap_chain(window);
	}

	static void destroy_vk(Window *window)
	{
		vkDestroySwapchainKHR(window->vk_logical_device, window->swap_chain, nullptr);
		vkDestroyDevice(window->vk_logical_device, nullptr);
		vkDestroySurfaceKHR(window->vk_instance, window->vk_surface, nullptr);
		vk_destroy_debug_messenger(window);
		vkDestroyInstance(window->vk_instance, nullptr);
	}

	Window *open_window(LinearAllocator *allocator, DisplayMode mode, Monitor *monitor, const char *title, u32 width, u32 height)
	{
		ASSERT(glfwInit(), "Failed to initialize glfw");
		ASSERT(title != nullptr, "Invalid title.");

		auto *window                   = v_alloc<LinearAllocator, Window>(allocator);

		GLFWmonitor *monitor_param     = nullptr;
		GLFWwindow *window_param       = nullptr;

		const auto *monitor_video_mode = glfwGetVideoMode(get_monitor_or_primary(monitor));

		switch (mode)
		{
			case DisplayMode::WINDOWED:
				monitor_param = nullptr;
				window_param  = nullptr;

				if (width == 0 || height == 0)
				{
					width  = monitor_video_mode->width - 500;
					height = monitor_video_mode->height - 500;
				}
				// We will resize the window later so don't do anything just yet.
				glfwWindowHint(GLFW_VISIBLE, 0);

				break;
			case DisplayMode::BORDERLESS_WINDOWED:
				monitor_param = get_monitor_or_primary(monitor);

				glfwWindowHint(GLFW_RED_BITS, monitor_video_mode->redBits);
				glfwWindowHint(GLFW_GREEN_BITS, monitor_video_mode->greenBits);
				glfwWindowHint(GLFW_BLUE_BITS, monitor_video_mode->blueBits);
				glfwWindowHint(GLFW_REFRESH_RATE, monitor_video_mode->refreshRate);

				width  = monitor_video_mode->width;
				height = monitor_video_mode->height;
				break;
			case DisplayMode::FULLSCREEN:
				monitor_param = get_monitor_or_primary(monitor);
				if (width == 0 || height == 0)
				{
					width  = monitor_video_mode->width;
					height = monitor_video_mode->height;
				}
				break;
			default:
				break;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window->glfw = glfwCreateWindow(width, height, title, monitor_param, window_param);
		ASSERT(window->glfw != nullptr, "Failed to create glfw window.");

		if (mode == DisplayMode::WINDOWED)
		{
			glfwMaximizeWindow(window->glfw);
			glfwShowWindow(window->glfw);
		}

		glfwMakeContextCurrent(window->glfw);
		vk_init(window, title);

		return window;
	}

	void close_window(Window *window)
	{
		ASSERT(window != nullptr && window->glfw != nullptr, "Invalid window.");

		destroy_vk(window);
		glfwDestroyWindow(window->glfw);
		glfwTerminate();
	}

	bool window_should_close(Window *window) { return glfwWindowShouldClose(window->glfw); }

	void swap_buffers(Window *window) { glfwSwapBuffers(window->glfw); }

	void poll_events(Window *window) { glfwPollEvents(); }
} // namespace Vultr::Platform
