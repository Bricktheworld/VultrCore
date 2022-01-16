#include "../../platform.h"
#include "types/optional.h"

namespace Vultr
{
	namespace Platform
	{
		static constexpr const char *VALIDATION_LAYERS[] = {"VK_LAYER_KHRONOS_validation"};
		static constexpr u32 VALIDATION_LAYERS_COUNT     = sizeof(VALIDATION_LAYERS) / sizeof(const char *);
		static constexpr const char *DEVICE_EXTENSIONS[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		static constexpr u32 DEVICE_EXTENSION_COUNT      = sizeof(DEVICE_EXTENSIONS) / sizeof(const char *);
		static constexpr u32 MAX_FRAMES_IN_FLIGHT        = 2;

		struct RenderContext
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

			VkRenderPass render_pass         = nullptr;
			VkPipelineLayout pipeline_layout = nullptr;
			VkPipeline graphics_pipeline     = nullptr;
			VkCommandPool command_pool       = nullptr;

			VkQueue graphics_queue           = nullptr;
			VkQueue present_queue            = nullptr;

			Vector<VkFramebuffer> swap_chain_framebuffers{};
			Vector<VkCommandBuffer> command_buffers{};

			VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT]{};
			VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT]{};
			VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT]{};
			Vector<VkFence> images_in_flight{};
			u32 current_frame                        = 0;

			bool debug                               = true;
			VkDebugUtilsMessengerEXT debug_messenger = nullptr;
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

		// TODO(Brandon): Don't do this here.
		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_cb(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
													   const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
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

		static void vk_init_debug_messenger(RenderContext *c)
		{
			VkDebugUtilsMessengerCreateInfoEXT create_info{
				.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				.pfnUserCallback = debug_cb,
				.pUserData       = nullptr,
			};

			auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(c->instance, "vkCreateDebugUtilsMessengerEXT");
			PRODUCTION_ASSERT(vkCreateDebugUtilsMessengerEXT != nullptr, "Failed to load vkCreateDebugUtilsMessengerEXT!");
			PRODUCTION_ASSERT(vkCreateDebugUtilsMessengerEXT(c->instance, &create_info, nullptr, &c->debug_messenger) == VK_SUCCESS, "Failed to create vulkan debug messenger!");
		}

		static void vk_destroy_debug_messenger(RenderContext *c)
		{
			auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(c->instance, "vkDestroyDebugUtilsMessengerEXT");
			PRODUCTION_ASSERT(vkDestroyDebugUtilsMessengerEXT != nullptr, "Failed to load vkDestroyDebugUtilsMessengerEXT!");
			vkDestroyDebugUtilsMessengerEXT(c->instance, c->debug_messenger, nullptr);
		}

		static bool vk_is_complete(const VkQueueFamilyIndices &indices) { return indices.graphics_family && indices.present_family; }
		static bool vk_is_complete(const VkSwapChainSupportDetails &details) { return !details.present_modes.empty() && !details.formats.empty(); }

		static VkQueueFamilyIndices vk_find_queue_families(VkPhysicalDevice vk_physical_device, RenderContext *c)
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
				vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, i, c->surface, &present_support);
				if (present_support)
				{
					indices.present_family = i;
				}

				if (vk_is_complete(indices))
					break;
			}

			return indices;
		}

		static VkSwapChainSupportDetails vk_query_swap_chain_support(VkPhysicalDevice device, RenderContext *c)
		{
			VkSwapChainSupportDetails details{};
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, c->surface, &details.capabilities);

			u32 format_count;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, c->surface, &format_count, nullptr);

			if (format_count != 0)
			{
				details.formats.resize(format_count);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, c->surface, &format_count, &details.formats[0]);
			}

			u32 present_modes_count;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, c->surface, &present_modes_count, nullptr);

			if (present_modes_count != 0)
			{
				details.present_modes.resize(present_modes_count);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, c->surface, &present_modes_count, &details.present_modes[0]);
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

		static u32 vk_rate_device_suitability(VkPhysicalDevice device, RenderContext *c)
		{
			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceProperties(device, &properties);
			vkGetPhysicalDeviceFeatures(device, &features);

			u32 score    = 0;

			auto indices = vk_find_queue_families(device, c);
			if (!vk_is_complete(indices))
				return 0;

			if (!vk_check_device_extension_support(device))
				return 0;

			auto details = vk_query_swap_chain_support(device, c);
			if (!vk_is_complete(details))
				return 0;

			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				score += 1000;
			}

			score += properties.limits.maxImageDimension2D;
			return score;
		}

		static void vk_pick_physical_device(RenderContext *c)
		{
			u32 device_count = 0;
			vkEnumeratePhysicalDevices(c->instance, &device_count, nullptr);

			PRODUCTION_ASSERT(device_count != 0, "No GPUs on this machine support Vulkan!");

			VkPhysicalDevice devices[device_count];
			vkEnumeratePhysicalDevices(c->instance, &device_count, devices);

			u32 best_score               = 0;
			VkPhysicalDevice best_device = nullptr;
			for (auto device : devices)
			{
				u32 score = vk_rate_device_suitability(device, c);
				if (score > best_score)
				{
					best_score  = score;
					best_device = device;
				}
			}

			PRODUCTION_ASSERT(best_score != 0 && best_device != nullptr, "No GPUs on this machine are supported!");
			c->physical_device = best_device;
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

		static VkExtent2D vk_pick_swap_extent(const Window *window, const VkSurfaceCapabilitiesKHR &capabilities)
		{
			if (capabilities.currentExtent.width != U32Max)
			{
				return capabilities.currentExtent;
			}
			else
			{
				int width, height;
				glfwGetFramebufferSize(window->glfw, &width, &height);

				VkExtent2D extent = {static_cast<u32>(width), static_cast<u32>(height)};

				extent.width      = Vultr::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				extent.height     = Vultr::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return extent;
			}
		}

		static void vk_init_swap_chain(const Window *window, RenderContext *c)
		{
			auto details        = vk_query_swap_chain_support(c->physical_device, c);

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
				.surface          = c->surface,
				.minImageCount    = image_count,
				.imageFormat      = surface_format.format,
				.imageColorSpace  = surface_format.colorSpace,
				.imageExtent      = extent,
				.imageArrayLayers = 1,
				.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			};

			auto indices               = vk_find_queue_families(c->physical_device, c);
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

			PRODUCTION_ASSERT(vkCreateSwapchainKHR(c->device, &create_info, nullptr, &c->swap_chain) == VK_SUCCESS, "Failed to create Vulkan swap chain!");
			vkGetSwapchainImagesKHR(c->device, c->swap_chain, &image_count, nullptr);

			c->swap_chain_images.resize(image_count);
			vkGetSwapchainImagesKHR(c->device, c->swap_chain, &image_count, &c->swap_chain_images[0]);

			c->swap_chain_image_format = surface_format.format;
			c->swap_chain_extent       = extent;
		}

		static void vk_init_image_views(RenderContext *c)
		{
			c->swap_chain_image_views.resize(c->swap_chain_images.size());

			for (size_t i = 0; i < c->swap_chain_image_views.size(); i++)
			{
				VkImageViewCreateInfo create_info{
					.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.image    = c->swap_chain_images[i],
					.viewType = VK_IMAGE_VIEW_TYPE_2D,
					.format   = c->swap_chain_image_format,
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
				PRODUCTION_ASSERT(vkCreateImageView(c->device, &create_info, nullptr, &c->swap_chain_image_views[i]) == VK_SUCCESS, "Failed to create Vulkan image view!");
			}
		}

		static void vk_init_logical_device(RenderContext *c)
		{
			auto indices       = vk_find_queue_families(c->physical_device, c);

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
			PRODUCTION_ASSERT(vkCreateDevice(c->physical_device, &create_info, nullptr, &c->device) == VK_SUCCESS, "Failed to create vulkan logical device!");
			vkGetDeviceQueue(c->device, indices.graphics_family.value(), 0, &c->graphics_queue);
			vkGetDeviceQueue(c->device, indices.present_family.value(), 0, &c->present_queue);
		}

		static void vk_init_surface(const Window *window, RenderContext *c)
		{
			PRODUCTION_ASSERT(glfwCreateWindowSurface(c->instance, window->glfw, nullptr, &c->surface) == VK_SUCCESS, "Failed to create vulkan c surface!");
		}

		static VkShaderModule vk_init_shader_module(RenderContext *c, const Buffer &src)
		{
			VkShaderModuleCreateInfo create_info{};
			create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			create_info.codeSize = src.size();
			create_info.pCode    = reinterpret_cast<const u32 *>(src.storage);
			VkShaderModule shader_module;
			ASSERT(vkCreateShaderModule(c->device, &create_info, nullptr, &shader_module) == VK_SUCCESS, "Failed to init vulkan shader!");
			return shader_module;
		}

		static void vk_init_render_pass(RenderContext *c)
		{
			VkAttachmentDescription color_attachment{
				.format         = c->swap_chain_image_format,
				.samples        = VK_SAMPLE_COUNT_1_BIT,
				.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			};

			VkAttachmentReference color_attachment_ref{
				.attachment = 0,
				.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};

			VkSubpassDescription subpass{
				.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments    = &color_attachment_ref,
			};

			VkSubpassDependency dependency{
				.srcSubpass    = VK_SUBPASS_EXTERNAL,
				.dstSubpass    = 0,
				.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			};

			VkRenderPassCreateInfo render_pass_info{
				.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.attachmentCount = 1,
				.pAttachments    = &color_attachment,
				.subpassCount    = 1,
				.pSubpasses      = &subpass,
				.dependencyCount = 1,
				.pDependencies   = &dependency,
			};

			PRODUCTION_ASSERT(vkCreateRenderPass(c->device, &render_pass_info, nullptr, &c->render_pass) == VK_SUCCESS, "Failed to create render pass!");
		}

		// TODO(Brandon): Definitely don't do this here.
		static void vk_init_graphics_pipeline(RenderContext *c)
		{
			Buffer vert_shader_src;
			fread_all(Path("./build/shaders/basic_vert.spv"), &vert_shader_src);

			Buffer frag_shader_src;
			fread_all(Path("./build/shaders/basic_frag.spv"), &frag_shader_src);

			auto vert_shader_module                  = vk_init_shader_module(c, vert_shader_src);
			auto frag_shader_module                  = vk_init_shader_module(c, frag_shader_src);

			VkPipelineShaderStageCreateInfo stages[] = {{
															.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
															.stage  = VK_SHADER_STAGE_VERTEX_BIT,
															.module = vert_shader_module,
															.pName  = "main",
														},
														{
															.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
															.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
															.module = frag_shader_module,
															.pName  = "main",
														}};

			VkPipelineVertexInputStateCreateInfo vertex_input_info{
				.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.vertexBindingDescriptionCount   = 0,
				.pVertexBindingDescriptions      = nullptr,
				.vertexAttributeDescriptionCount = 0,
				.pVertexAttributeDescriptions    = nullptr,
			};

			VkPipelineInputAssemblyStateCreateInfo input_assembly{
				.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.primitiveRestartEnable = VK_FALSE,
			};

			VkViewport viewport{
				.x        = 0.0f,
				.y        = 0.0f,
				.width    = static_cast<f32>(c->swap_chain_extent.width),
				.height   = static_cast<f32>(c->swap_chain_extent.height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f,
			};

			VkRect2D scissor{
				.offset = {0, 0},
				.extent = c->swap_chain_extent,
			};

			VkPipelineViewportStateCreateInfo viewport_state{
				.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.viewportCount = 1,
				.pViewports    = &viewport,
				.scissorCount  = 1,
				.pScissors     = &scissor,
			};

			VkPipelineRasterizationStateCreateInfo rasterizer{
				.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				.depthClampEnable        = VK_FALSE,
				.rasterizerDiscardEnable = VK_FALSE,
				.polygonMode             = VK_POLYGON_MODE_FILL,
				.cullMode                = VK_CULL_MODE_BACK_BIT,
				.frontFace               = VK_FRONT_FACE_CLOCKWISE,
				.depthBiasEnable         = VK_FALSE,
				.depthBiasConstantFactor = 0.0f,
				.depthBiasClamp          = 0.0f,
				.depthBiasSlopeFactor    = 0.0f,
				.lineWidth               = 1.0f,
			};

			VkPipelineMultisampleStateCreateInfo multisampling{
				.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
				.sampleShadingEnable   = VK_FALSE,
				.minSampleShading      = 1.0f,
				.pSampleMask           = nullptr,
				.alphaToCoverageEnable = VK_FALSE,
				.alphaToOneEnable      = VK_FALSE,
			};

			VkPipelineColorBlendAttachmentState color_blend_attachments{
				.blendEnable         = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
				.colorBlendOp        = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
				.alphaBlendOp        = VK_BLEND_OP_ADD,
				.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			};

			VkPipelineColorBlendStateCreateInfo color_blending{
				.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.logicOpEnable   = VK_FALSE,
				.logicOp         = VK_LOGIC_OP_COPY,
				.attachmentCount = 1,
				.pAttachments    = &color_blend_attachments,
				.blendConstants  = {0.0f, 0.0f, 0.0f, 0.0f},
			};

			VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

			VkPipelineDynamicStateCreateInfo dynamic_state{
				.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.dynamicStateCount = 2,
				.pDynamicStates    = dynamic_states,
			};

			VkPipelineLayoutCreateInfo pipeline_layout_info{
				.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount         = 0,
				.pSetLayouts            = nullptr,
				.pushConstantRangeCount = 0,
				.pPushConstantRanges    = nullptr,
			};

			PRODUCTION_ASSERT(vkCreatePipelineLayout(c->device, &pipeline_layout_info, nullptr, &c->pipeline_layout) == VK_SUCCESS, "");

			VkGraphicsPipelineCreateInfo pipeline_info{
				.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.stageCount          = 2,
				.pStages             = stages,
				.pVertexInputState   = &vertex_input_info,
				.pInputAssemblyState = &input_assembly,
				.pViewportState      = &viewport_state,
				.pRasterizationState = &rasterizer,
				.pMultisampleState   = &multisampling,
				.pDepthStencilState  = nullptr,
				.pColorBlendState    = &color_blending,
				.pDynamicState       = nullptr,
				.layout              = c->pipeline_layout,
				.renderPass          = c->render_pass,
				.subpass             = 0,
				.basePipelineHandle  = VK_NULL_HANDLE,
				.basePipelineIndex   = -1,
			};

			PRODUCTION_ASSERT(vkCreateGraphicsPipelines(c->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c->graphics_pipeline) == VK_SUCCESS, "Failed to create graphics pipeline!");

			vkDestroyShaderModule(c->device, frag_shader_module, nullptr);
			vkDestroyShaderModule(c->device, vert_shader_module, nullptr);
		}

		static void vk_init_framebuffers(RenderContext *c)
		{
			c->swap_chain_framebuffers.resize(c->swap_chain_image_views.size());
			for (size_t i = 0; i < c->swap_chain_image_views.size(); i++)
			{
				VkImageView attachments[] = {c->swap_chain_image_views[i]};

				VkFramebufferCreateInfo framebuffer_info{
					.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.renderPass      = c->render_pass,
					.attachmentCount = 1,
					.pAttachments    = attachments,
					.width           = c->swap_chain_extent.width,
					.height          = c->swap_chain_extent.height,
					.layers          = 1,
				};

				PRODUCTION_ASSERT(vkCreateFramebuffer(c->device, &framebuffer_info, nullptr, &c->swap_chain_framebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer!");
			}
		}

		static void vk_init_command_pools(RenderContext *c)
		{
			auto indices = vk_find_queue_families(c->physical_device, c);

			VkCommandPoolCreateInfo pool_info{
				.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags            = 0,
				.queueFamilyIndex = indices.graphics_family.value(),
			};

			PRODUCTION_ASSERT(vkCreateCommandPool(c->device, &pool_info, nullptr, &c->command_pool) == VK_SUCCESS, "Failed to create command pool!");
		}

		// TODO(Brandon): Don't do this here.
		static void vk_init_command_buffers(RenderContext *c)
		{
			c->command_buffers.resize(c->swap_chain_framebuffers.size());

			VkCommandBufferAllocateInfo alloc_info{
				.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool        = c->command_pool,
				.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = (u32)c->command_buffers.size(),
			};

			PRODUCTION_ASSERT(vkAllocateCommandBuffers(c->device, &alloc_info, &c->command_buffers[0]) == VK_SUCCESS, "Failed to allocate command buffers!");

			for (size_t i = 0; i < c->command_buffers.size(); i++)
			{
				const auto &command_buffer = c->command_buffers[i];
				VkCommandBufferBeginInfo begin_info{
					.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					.flags            = 0,
					.pInheritanceInfo = nullptr,
				};

				PRODUCTION_ASSERT(vkBeginCommandBuffer(command_buffer, &begin_info) == VK_SUCCESS, "Failed to begin recording command buffer!");

				VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
				VkRenderPassBeginInfo render_pass_info{
					.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass  = c->render_pass,
					.framebuffer = c->swap_chain_framebuffers[i],
					.renderArea =
						{
							.offset = {0, 0},
							.extent = c->swap_chain_extent,
						},
					.clearValueCount = 1,
					.pClearValues    = &clear_color,
				};

				vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, c->graphics_pipeline);
				vkCmdDraw(command_buffer, 3, 1, 0, 0);

				vkCmdEndRenderPass(command_buffer);

				PRODUCTION_ASSERT(vkEndCommandBuffer(command_buffer) == VK_SUCCESS, "Failed to record command buffer!");
			}
		}

		static void vk_init_concurrency(RenderContext *c)
		{
			c->images_in_flight.resize(c->swap_chain_images.size());
			memset(&c->images_in_flight[0], 0, c->swap_chain_images.size() * sizeof(VkFence));

			VkSemaphoreCreateInfo semaphore_info{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

			VkFenceCreateInfo fence_info{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT,
			};

			for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				PRODUCTION_ASSERT(vkCreateSemaphore(c->device, &semaphore_info, nullptr, &c->image_available_semaphores[i]) == VK_SUCCESS, "Failed to create image available semaphore!");
				PRODUCTION_ASSERT(vkCreateSemaphore(c->device, &semaphore_info, nullptr, &c->render_finished_semaphores[i]) == VK_SUCCESS, "Failed to create render finished semaphore!");
				PRODUCTION_ASSERT(vkCreateFence(c->device, &fence_info, nullptr, &c->in_flight_fences[i]) == VK_SUCCESS, "Failed to create in flight fence!");
			}
		}

		static void vk_init(const Window *window, RenderContext *c)
		{
			VkApplicationInfo vk_application_info{
				.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName   = get_window_title(window).c_str(),
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

			if (c->debug)
			{
				vk_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			vk_instance_create_info.enabledExtensionCount   = vk_extensions.size();
			vk_instance_create_info.ppEnabledExtensionNames = &vk_extensions[0];

			if (c->debug)
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

			PRODUCTION_ASSERT(vkCreateInstance(&vk_instance_create_info, nullptr, &c->instance) == VK_SUCCESS, "Failed to init vulkan.");
			vk_init_debug_messenger(c);
			vk_init_surface(window, c);
			vk_pick_physical_device(c);
			vk_init_logical_device(c);
			vk_init_swap_chain(window, c);
			vk_init_image_views(c);
			vk_init_render_pass(c);
			vk_init_graphics_pipeline(c);
			vk_init_framebuffers(c);
			vk_init_command_pools(c);
			vk_init_command_buffers(c);
			vk_init_concurrency(c);
		}

		RenderContext *init_render_context(const Window *window, bool debug)
		{
			auto *c = static_cast<RenderContext *>(persist_alloc(sizeof(RenderContext)));
			PRODUCTION_ASSERT(c != nullptr, "Failed to allocate render context!");
			new (c) RenderContext();
			c->debug = debug;

			vk_init(window, c);
			return c;
		}

		void draw_frame(RenderContext *c)
		{
			vkWaitForFences(c->device, 1, &c->in_flight_fences[c->current_frame], VK_TRUE, UINT64_MAX);

			u32 image_index;
			vkAcquireNextImageKHR(c->device, c->swap_chain, U64Max, c->image_available_semaphores[c->current_frame], VK_NULL_HANDLE, &image_index);

			if (c->images_in_flight[image_index] != VK_NULL_HANDLE)
			{
				vkWaitForFences(c->device, 1, &c->images_in_flight[image_index], VK_TRUE, U64Max);
			}

			c->images_in_flight[image_index]   = c->in_flight_fences[c->current_frame];

			VkSemaphore wait_semaphores[]      = {c->image_available_semaphores[c->current_frame]};
			VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
			VkSemaphore signal_semaphores[]    = {c->render_finished_semaphores[c->current_frame]};

			VkSubmitInfo submit_info{
				.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.waitSemaphoreCount   = 1,
				.pWaitSemaphores      = wait_semaphores,
				.pWaitDstStageMask    = wait_stages,
				.commandBufferCount   = 1,
				.pCommandBuffers      = &c->command_buffers[image_index],
				.signalSemaphoreCount = 1,
				.pSignalSemaphores    = signal_semaphores,
			};

			vkResetFences(c->device, 1, &c->in_flight_fences[c->current_frame]);
			PRODUCTION_ASSERT(vkQueueSubmit(c->graphics_queue, 1, &submit_info, c->in_flight_fences[c->current_frame]) == VK_SUCCESS, "Failed to submit queue!");

			VkSwapchainKHR swapChains[] = {c->swap_chain};
			VkPresentInfoKHR present_info{
				.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores    = signal_semaphores,
				.swapchainCount     = 1,
				.pSwapchains        = swapChains,
				.pImageIndices      = &image_index,
				.pResults           = nullptr,
			};
			vkQueuePresentKHR(c->present_queue, &present_info);

			c->current_frame = (c->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
		}

		void destroy_render_context(RenderContext *c)
		{
			// Make sure don't destroy anything that is in use.
			vkDeviceWaitIdle(c->device);

			for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				vkDestroySemaphore(c->device, c->render_finished_semaphores[i], nullptr);
				vkDestroySemaphore(c->device, c->image_available_semaphores[i], nullptr);
				vkDestroyFence(c->device, c->in_flight_fences[i], nullptr);
			}
			vkDestroyCommandPool(c->device, c->command_pool, nullptr);
			for (auto *framebuffer : c->swap_chain_framebuffers)
			{
				vkDestroyFramebuffer(c->device, framebuffer, nullptr);
			}
			vkDestroyPipeline(c->device, c->graphics_pipeline, nullptr);
			vkDestroyPipelineLayout(c->device, c->pipeline_layout, nullptr);
			vkDestroyRenderPass(c->device, c->render_pass, nullptr);
			for (auto *view : c->swap_chain_image_views)
			{
				vkDestroyImageView(c->device, view, nullptr);
			}
			vkDestroySwapchainKHR(c->device, c->swap_chain, nullptr);
			vkDestroyDevice(c->device, nullptr);
			vkDestroySurfaceKHR(c->instance, c->surface, nullptr);
			vk_destroy_debug_messenger(c);
			vkDestroyInstance(c->instance, nullptr);
		}
	} // namespace Platform
} // namespace Vultr