#include "device.h"

namespace Vultr
{
	namespace Vulkan
	{

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
		QueueFamilyIndices find_queue_families(Device *d) { return find_queue_families(d->surface, d->physical_device); }

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
		SwapChainSupportDetails query_swap_chain_support(Device *d) { return query_swap_chain_support(d->surface, d->physical_device); };

		size_t min_ubo_alignment(Device *d) { return d->properties.limits.minUniformBufferOffsetAlignment; }

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
			vkGetPhysicalDeviceProperties(d->physical_device, &d->properties);
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

		static void init_memory_allocator(Device *d)
		{
			VmaAllocatorCreateInfo info = {
				.physicalDevice = d->physical_device,
				.device         = d->device,
				.instance       = d->instance,
			};
			VK_CHECK(vmaCreateAllocator(&info, &d->allocator));
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
			init_memory_allocator(&device);
			return device;
		}

		static void destroy_debug_messenger(Device *d)
		{
			auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(d->instance, "vkDestroyDebugUtilsMessengerEXT");
			PRODUCTION_ASSERT(vkDestroyDebugUtilsMessengerEXT != nullptr, "Failed to load vkDestroyDebugUtilsMessengerEXT!");
			vkDestroyDebugUtilsMessengerEXT(d->instance, d->debug_messenger, nullptr);
		}

		void graphics_queue_submit(Device *d, u32 submit_count, const VkSubmitInfo *p_submits, VkFence fence)
		{
			Platform::Lock lock(d->graphics_queue_mutex);
			VK_CHECK(vkQueueSubmit(d->graphics_queue, submit_count, p_submits, fence));
		}

		void wait_idle(Device *d) { vkDeviceWaitIdle(d->device); }

		VkFormat get_supported_depth_format(Device *d)
		{
			VkFormat depth_formats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};
			for (auto format : depth_formats)
			{
				VkFormatProperties properties;
				vkGetPhysicalDeviceFormatProperties(d->physical_device, format, &properties);
				if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					return format;
				}
			}
			THROW("No supported depth formats found!");
		}

		void destroy_device(Device *d)
		{
			vmaDestroyAllocator(d->allocator);
			vkDestroyDevice(d->device, nullptr);

			vkDestroySurfaceKHR(d->instance, d->surface, nullptr);
			destroy_debug_messenger(d);
			vkDestroyInstance(d->instance, nullptr);
		}
	} // namespace Vulkan
} // namespace Vultr
