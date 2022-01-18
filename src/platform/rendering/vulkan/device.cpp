#include "device.h"
#include "math/clamp.h"

namespace Vultr
{
	namespace Vulkan
	{
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

		static void init_debug_messenger(Device *d, PFN_vkDebugUtilsMessengerCallbackEXT debug_cb)
		{
			VkDebugUtilsMessengerCreateInfoEXT create_info{
				.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				.pfnUserCallback = debug_cb,
				.pUserData       = nullptr,
			};

			auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(d->instance, "vkCreateDebugUtilsMessengerEXT");
			PRODUCTION_ASSERT(vkCreateDebugUtilsMessengerEXT != nullptr, "Failed to load vkCreateDebugUtilsMessengerEXT!");
			PRODUCTION_ASSERT(vkCreateDebugUtilsMessengerEXT(d->instance, &create_info, nullptr, &d->debug_messenger) == VK_SUCCESS, "Failed to create vulkan debug messenger!");
		}

		static bool has_validation(VkLayerProperties available_layers[], size_t available_layers_count)
		{
			for (str layer : VALIDATION_LAYERS)
			{
				bool layer_found = false;

				for (size_t j = 0; j < available_layers_count; j++)
				{
					if (strcmp(layer, available_layers[j].layerName) == 0)
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

		static void init_surface(Device *d, const Platform::Window *window)
		{
			PRODUCTION_ASSERT(glfwCreateWindowSurface(d->instance, static_cast<GLFWwindow *>(Platform::get_window_implementation(window)), nullptr, &d->surface) == VK_SUCCESS, "Failed to create vulkan c surface!");
		}

		static bool is_complete(const QueueFamilyIndices &indices) { return indices.graphics_family && indices.present_family; }
		static bool is_complete(const SwapChainSupportDetails &details) { return !details.present_modes.empty() && !details.formats.empty(); }

		static QueueFamilyIndices find_queue_families(VkSurfaceKHR surface, VkPhysicalDevice physical_device)
		{
			QueueFamilyIndices indices{};

			u32 count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);

			VkQueueFamilyProperties properties[count];
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, properties);

			for (u32 i = 0; i < count; i++)
			{
				if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					indices.graphics_family = i;
				}

				VkBool32 present_support = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
				if (present_support)
				{
					indices.present_family = i;
				}

				if (is_complete(indices))
					break;
			}

			return indices;
		}
		static QueueFamilyIndices find_queue_families(Device *d) { return find_queue_families(d->surface, d->physical_device); }

		static bool check_device_extension_support(VkPhysicalDevice device)
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

		static SwapChainSupportDetails query_swap_chain_support(VkSurfaceKHR surface, VkPhysicalDevice device)
		{
			SwapChainSupportDetails details{};
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

			u32 format_count;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

			if (format_count != 0)
			{
				details.formats.resize(format_count);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, &details.formats[0]);
			}

			u32 present_modes_count;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_modes_count, nullptr);

			if (present_modes_count != 0)
			{
				details.present_modes.resize(present_modes_count);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_modes_count, &details.present_modes[0]);
			}

			return details;
		}
		static SwapChainSupportDetails query_swap_chain_support(Device *d) { return query_swap_chain_support(d->surface, d->physical_device); };

		static u32 rate_device_suitability(VkSurfaceKHR surface, VkPhysicalDevice device)
		{
			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceProperties(device, &properties);
			vkGetPhysicalDeviceFeatures(device, &features);

			u32 score    = 0;

			auto indices = find_queue_families(surface, device);
			if (!is_complete(indices))
				return 0;

			if (!check_device_extension_support(device))
				return 0;

			auto details = query_swap_chain_support(surface, device);
			if (!is_complete(details))
				return 0;

			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				score += 1000;
			}

			score += properties.limits.maxImageDimension2D;
			return score;
		}

		static void pick_physical_device(Device *d)
		{
			u32 device_count = 0;
			vkEnumeratePhysicalDevices(d->instance, &device_count, nullptr);

			PRODUCTION_ASSERT(device_count != 0, "No GPUs on this machine support Vulkan!");

			VkPhysicalDevice devices[device_count];
			vkEnumeratePhysicalDevices(d->instance, &device_count, devices);

			u32 best_score               = 0;
			VkPhysicalDevice best_device = nullptr;
			for (auto device : devices)
			{
				u32 score = rate_device_suitability(d->surface, device);
				if (score > best_score)
				{
					best_score  = score;
					best_device = device;
				}
			}

			PRODUCTION_ASSERT(best_score != 0 && best_device != nullptr, "No GPUs on this machine are supported!");
			d->physical_device = best_device;
		}

		static void init_logical_device(Device *d)
		{
			auto indices       = find_queue_families(d);

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
				.queueCreateInfoCount    = static_cast<u32>(queue_indices.size()),
				.pQueueCreateInfos       = queue_create_infos,
				.enabledLayerCount       = VALIDATION_LAYERS.size(),
				.ppEnabledLayerNames     = &VALIDATION_LAYERS[0],
				.enabledExtensionCount   = DEVICE_EXTENSIONS.size(),
				.ppEnabledExtensionNames = &DEVICE_EXTENSIONS[0],
				.pEnabledFeatures        = &device_features,
			};
			PRODUCTION_ASSERT(vkCreateDevice(d->physical_device, &create_info, nullptr, &d->device) == VK_SUCCESS, "Failed to create vulkan logical device!");
			vkGetDeviceQueue(d->device, indices.graphics_family.value(), 0, &d->graphics_queue);
			vkGetDeviceQueue(d->device, indices.present_family.value(), 0, &d->present_queue);
		}

		static VkExtent2D pick_swap_extent(const Platform::Window *window, const VkSurfaceCapabilitiesKHR &capabilities)
		{
			if (capabilities.currentExtent.width != U32Max)
			{
				return capabilities.currentExtent;
			}
			else
			{
				int width, height;
				glfwGetFramebufferSize(static_cast<GLFWwindow *>(Platform::get_window_implementation(window)), &width, &height);

				VkExtent2D extent = {static_cast<u32>(width), static_cast<u32>(height)};

				extent.width      = Vultr::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				extent.height     = Vultr::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return extent;
			}
		}

		static VkSurfaceFormatKHR pick_swap_chain_surface_format(const Vector<VkSurfaceFormatKHR> &available_formats)
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

		static VkPresentModeKHR pick_present_mode(const Vector<VkPresentModeKHR> &available_modes)
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

		static void init_swap_chain(Device *d, const Platform::Window *window)
		{
			auto details        = query_swap_chain_support(d);

			auto surface_format = pick_swap_chain_surface_format(details.formats);
			auto present_mode   = pick_present_mode(details.present_modes);
			auto extent         = pick_swap_extent(window, details.capabilities);

			u32 image_count     = details.capabilities.minImageCount + 1;
			if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount)
			{
				image_count = details.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR create_info{
				.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.surface          = d->surface,
				.minImageCount    = image_count,
				.imageFormat      = surface_format.format,
				.imageColorSpace  = surface_format.colorSpace,
				.imageExtent      = extent,
				.imageArrayLayers = 1,
				.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			};

			auto indices               = find_queue_families(d);
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

			PRODUCTION_ASSERT(vkCreateSwapchainKHR(d->device, &create_info, nullptr, &d->swap_chain) == VK_SUCCESS, "Failed to create Vulkan swap chain!");
			vkGetSwapchainImagesKHR(d->device, d->swap_chain, &image_count, nullptr);

			d->swap_chain_images.resize(image_count);
			vkGetSwapchainImagesKHR(d->device, d->swap_chain, &image_count, &d->swap_chain_images[0]);

			d->swap_chain_image_format = surface_format.format;
			d->swap_chain_extent       = extent;
		}

		static void init_image_views(Device *d)
		{
			d->swap_chain_image_views.resize(d->swap_chain_images.size());

			for (size_t i = 0; i < d->swap_chain_image_views.size(); i++)
			{
				VkImageViewCreateInfo create_info{
					.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.image    = d->swap_chain_images[i],
					.viewType = VK_IMAGE_VIEW_TYPE_2D,
					.format   = d->swap_chain_image_format,
					.components =
						{
							.r = VK_COMPONENT_SWIZZLE_IDENTITY,
							.g = VK_COMPONENT_SWIZZLE_IDENTITY,
							.b = VK_COMPONENT_SWIZZLE_IDENTITY,
							.a = VK_COMPONENT_SWIZZLE_IDENTITY,
						},
					.subresourceRange =
						{
							.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel   = 0,
							.levelCount     = 1,
							.baseArrayLayer = 0,
							.layerCount     = 1,
						},
				};
				PRODUCTION_ASSERT(vkCreateImageView(d->device, &create_info, nullptr, &d->swap_chain_image_views[i]) == VK_SUCCESS, "Failed to create Vulkan image view!");
			}
		}

		static void init_command_pools(Device *d)
		{
			auto indices = find_queue_families(d);

			VkCommandPoolCreateInfo pool_info{
				.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags            = 0,
				.queueFamilyIndex = indices.graphics_family.value(),
			};

			PRODUCTION_ASSERT(vkCreateCommandPool(d->device, &pool_info, nullptr, &d->graphics_command_pool) == VK_SUCCESS, "Failed to create command pool!");
		}

		Device init_device(const Platform::Window *window, bool debug, PFN_vkDebugUtilsMessengerCallbackEXT debug_cb)
		{
			VkApplicationInfo application_info{
				.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName   = get_window_title(window).c_str(),
				.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
				.pEngineName        = "Vultr Game Engine",
				.engineVersion      = VK_MAKE_VERSION(1, 0, 0),
				.apiVersion         = VK_API_VERSION_1_0,
			};

			VkInstanceCreateInfo instance_create_info{
				.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pApplicationInfo = &application_info,
			};

			u32 glfw_extension_count     = 0;
			const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
			Vector extensions(glfw_extensions, glfw_extension_count);

			if (debug)
			{
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			instance_create_info.enabledExtensionCount   = extensions.size();
			instance_create_info.ppEnabledExtensionNames = &extensions[0];

			if (debug)
			{
				u32 layer_count;
				vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

				VkLayerProperties available_validation_layers[layer_count];
				vkEnumerateInstanceLayerProperties(&layer_count, available_validation_layers);

				PRODUCTION_ASSERT(has_validation(available_validation_layers, layer_count), "Vulkan failed to find validation layers!");

				instance_create_info.enabledLayerCount   = 1;
				instance_create_info.ppEnabledLayerNames = &VALIDATION_LAYERS[0];
			}
			else
			{
				instance_create_info.enabledLayerCount = 0;
			}

			Device device;
			PRODUCTION_ASSERT(vkCreateInstance(&instance_create_info, nullptr, &device.instance) == VK_SUCCESS, "Failed to init vulkan.");
			init_debug_messenger(&device, debug_cb);
			init_surface(&device, window);
			pick_physical_device(&device);
			init_logical_device(&device);
			init_swap_chain(&device, window);
			init_image_views(&device);
			init_command_pools(&device);
			return device;
		}

		static void destroy_debug_messenger(Device *d)
		{
			auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(d->instance, "vkDestroyDebugUtilsMessengerEXT");
			PRODUCTION_ASSERT(vkDestroyDebugUtilsMessengerEXT != nullptr, "Failed to load vkDestroyDebugUtilsMessengerEXT!");
			vkDestroyDebugUtilsMessengerEXT(d->instance, d->debug_messenger, nullptr);
		}

		static void destroy_swapchain(Device *d)
		{
			//			for (auto *framebuffer : c->swap_chain_framebuffers)
			//			{
			//				vkDestroyFramebuffer(c->device, framebuffer, nullptr);
			//			}
			//
			//			vkFreeCommandBuffers(c->device, c->command_pool, static_cast<u32>(c->command_buffers.size()), &c->command_buffers[0]);
			//
			//			vkDestroyPipeline(c->device, c->graphics_pipeline, nullptr);
			//			vkDestroyPipelineLayout(c->device, c->pipeline_layout, nullptr);
			//			vkDestroyRenderPass(c->device, c->render_pass, nullptr);
			//
			for (auto *view : d->swap_chain_image_views)
			{
				vkDestroyImageView(d->device, view, nullptr);
			}

			vkDestroySwapchainKHR(d->device, d->swap_chain, nullptr);
		}

		void recreate_swapchain(Device *d, const Platform::Window *window)
		{
			// Minimization
			int width = 0, height = 0;

			auto *glfw = static_cast<GLFWwindow *>(Platform::get_window_implementation(window));
			glfwGetFramebufferSize(glfw, &width, &height);
			while (width == 0 || height == 0)
			{
				glfwGetFramebufferSize(glfw, &width, &height);
				glfwWaitEvents();
			}

			wait_idle(d);

			destroy_swapchain(d);

			init_swap_chain(d, window);
			init_image_views(d);
		}

		void wait_idle(Device *d) { vkDeviceWaitIdle(d->device); }

		void destroy_device(Device *d)
		{
			destroy_swapchain(d);
			vkDestroyCommandPool(d->device, d->graphics_command_pool, nullptr);
			vkDestroyDevice(d->device, nullptr);

			vkDestroySurfaceKHR(d->instance, d->surface, nullptr);
			destroy_debug_messenger(d);
			vkDestroyInstance(d->instance, nullptr);
		}
	} // namespace Vulkan
} // namespace Vultr