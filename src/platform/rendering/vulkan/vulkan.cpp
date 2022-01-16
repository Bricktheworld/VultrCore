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

		struct RenderContext
		{
			VkInstance vk_instance              = nullptr;
			VkPhysicalDevice vk_physical_device = nullptr;

			VkDevice vk_logical_device          = nullptr;
			VkSurfaceKHR vk_surface             = nullptr;
			VkSwapchainKHR vk_swap_chain        = nullptr;
			VkFormat vk_swap_chain_image_format{};
			VkExtent2D vk_swap_chain_extent{};
			Vector<VkImage> vk_swap_chain_images{};
			Vector<VkImageView> vk_swap_chain_image_views{};

			VkRenderPass vk_render_pass         = nullptr;
			VkPipelineLayout vk_pipeline_layout = nullptr;
			VkPipeline vk_graphics_pipeline     = nullptr;
			VkCommandPool vk_command_pool       = nullptr;

			VkQueue vk_graphics_queue           = nullptr;
			VkQueue vk_present_queue            = nullptr;

			Vector<VkFramebuffer> vk_swap_chain_framebuffers{};
			Vector<VkCommandBuffer> vk_command_buffers{};

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

			auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(c->vk_instance, "vkCreateDebugUtilsMessengerEXT");
			PRODUCTION_ASSERT(vkCreateDebugUtilsMessengerEXT != nullptr, "Failed to load vkCreateDebugUtilsMessengerEXT!");
			PRODUCTION_ASSERT(vkCreateDebugUtilsMessengerEXT(c->vk_instance, &create_info, nullptr, &c->vk_debug_messenger) == VK_SUCCESS, "Failed to create vulkan debug messenger!");
		}

		static void vk_destroy_debug_messenger(RenderContext *c)
		{
			auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(c->vk_instance, "vkDestroyDebugUtilsMessengerEXT");
			PRODUCTION_ASSERT(vkDestroyDebugUtilsMessengerEXT != nullptr, "Failed to load vkDestroyDebugUtilsMessengerEXT!");
			vkDestroyDebugUtilsMessengerEXT(c->vk_instance, c->vk_debug_messenger, nullptr);
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
				vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, i, c->vk_surface, &present_support);
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
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, c->vk_surface, &details.capabilities);

			u32 format_count;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, c->vk_surface, &format_count, nullptr);

			if (format_count != 0)
			{
				details.formats.resize(format_count);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, c->vk_surface, &format_count, &details.formats[0]);
			}

			u32 present_modes_count;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, c->vk_surface, &present_modes_count, nullptr);

			if (present_modes_count != 0)
			{
				details.present_modes.resize(present_modes_count);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, c->vk_surface, &present_modes_count, &details.present_modes[0]);
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
			vkEnumeratePhysicalDevices(c->vk_instance, &device_count, nullptr);

			PRODUCTION_ASSERT(device_count != 0, "No GPUs on this machine support Vulkan!");

			VkPhysicalDevice devices[device_count];
			vkEnumeratePhysicalDevices(c->vk_instance, &device_count, devices);

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
			c->vk_physical_device = best_device;
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

				VkExtent2D actualExtent = {static_cast<u32>(width), static_cast<u32>(height)};

				actualExtent.width      = Vultr::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				actualExtent.height     = Vultr::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return actualExtent;
			}
		}

		static void vk_init_swap_chain(const Window *window, RenderContext *c)
		{
			auto details        = vk_query_swap_chain_support(c->vk_physical_device, c);

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
				.surface          = c->vk_surface,
				.minImageCount    = image_count,
				.imageFormat      = surface_format.format,
				.imageColorSpace  = surface_format.colorSpace,
				.imageExtent      = extent,
				.imageArrayLayers = 1,
				.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			};

			auto indices               = vk_find_queue_families(c->vk_physical_device, c);
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

			PRODUCTION_ASSERT(vkCreateSwapchainKHR(c->vk_logical_device, &create_info, nullptr, &c->vk_swap_chain) == VK_SUCCESS, "Failed to create Vulkan swap chain!");
			vkGetSwapchainImagesKHR(c->vk_logical_device, c->vk_swap_chain, &image_count, nullptr);

			c->vk_swap_chain_images.resize(image_count);
			vkGetSwapchainImagesKHR(c->vk_logical_device, c->vk_swap_chain, &image_count, &c->vk_swap_chain_images[0]);

			c->vk_swap_chain_image_format = surface_format.format;
			c->vk_swap_chain_extent       = extent;
		}

		static void vk_init_image_views(RenderContext *c)
		{
			c->vk_swap_chain_image_views.resize(c->vk_swap_chain_images.size());

			for (size_t i = 0; i < c->vk_swap_chain_image_views.size(); i++)
			{
				VkImageViewCreateInfo create_info{
					.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.image    = c->vk_swap_chain_images[i],
					.viewType = VK_IMAGE_VIEW_TYPE_2D,
					.format   = c->vk_swap_chain_image_format,
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
				PRODUCTION_ASSERT(vkCreateImageView(c->vk_logical_device, &create_info, nullptr, &c->vk_swap_chain_image_views[i]) == VK_SUCCESS, "Failed to create Vulkan image view!");
			}
		}

		static void vk_init_logical_device(RenderContext *c)
		{
			auto indices       = vk_find_queue_families(c->vk_physical_device, c);

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
			PRODUCTION_ASSERT(vkCreateDevice(c->vk_physical_device, &create_info, nullptr, &c->vk_logical_device) == VK_SUCCESS, "Failed to create vulkan logical device!");
			vkGetDeviceQueue(c->vk_logical_device, indices.graphics_family.value(), 0, &c->vk_graphics_queue);
			vkGetDeviceQueue(c->vk_logical_device, indices.present_family.value(), 0, &c->vk_present_queue);
		}

		static void vk_init_surface(const Window *window, RenderContext *c)
		{
			PRODUCTION_ASSERT(glfwCreateWindowSurface(c->vk_instance, window->glfw, nullptr, &c->vk_surface) == VK_SUCCESS, "Failed to create vulkan c surface!");
		}

		static VkShaderModule vk_init_shader_module(RenderContext *c, const Buffer &src)
		{
			VkShaderModuleCreateInfo create_info{};
			create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			create_info.codeSize = src.size();
			create_info.pCode    = reinterpret_cast<const u32 *>(src.storage);
			VkShaderModule shader_module;
			ASSERT(vkCreateShaderModule(c->vk_logical_device, &create_info, nullptr, &shader_module) == VK_SUCCESS, "Failed to init vulkan shader!");
			return shader_module;
		}

		static void vk_init_render_pass(RenderContext *c)
		{
			VkAttachmentDescription color_attachment{
				.format         = c->vk_swap_chain_image_format,
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

			VkRenderPassCreateInfo render_pass_info{
				.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.attachmentCount = 1,
				.pAttachments    = &color_attachment,
				.subpassCount    = 1,
				.pSubpasses      = &subpass,
			};

			PRODUCTION_ASSERT(vkCreateRenderPass(c->vk_logical_device, &render_pass_info, nullptr, &c->vk_render_pass) == VK_SUCCESS, "Failed to create render pass!");
		}

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
				.width    = static_cast<f32>(c->vk_swap_chain_extent.width),
				.height   = static_cast<f32>(c->vk_swap_chain_extent.height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f,
			};

			VkRect2D scissor{
				.offset = {0, 0},
				.extent = c->vk_swap_chain_extent,
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

			PRODUCTION_ASSERT(vkCreatePipelineLayout(c->vk_logical_device, &pipeline_layout_info, nullptr, &c->vk_pipeline_layout) == VK_SUCCESS, "");

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
				.layout              = c->vk_pipeline_layout,
				.renderPass          = c->vk_render_pass,
				.subpass             = 0,
				.basePipelineHandle  = VK_NULL_HANDLE,
				.basePipelineIndex   = -1,
			};

			PRODUCTION_ASSERT(vkCreateGraphicsPipelines(c->vk_logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c->vk_graphics_pipeline) == VK_SUCCESS, "Failed to create graphics pipeline!");

			vkDestroyShaderModule(c->vk_logical_device, frag_shader_module, nullptr);
			vkDestroyShaderModule(c->vk_logical_device, vert_shader_module, nullptr);
		}

		static void vk_init_framebuffers(RenderContext *c)
		{
			c->vk_swap_chain_framebuffers.resize(c->vk_swap_chain_image_views.size());
			for (size_t i = 0; i < c->vk_swap_chain_image_views.size(); i++)
			{
				VkImageView attachments[] = {c->vk_swap_chain_image_views[i]};

				VkFramebufferCreateInfo framebuffer_info{
					.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.renderPass      = c->vk_render_pass,
					.attachmentCount = 1,
					.pAttachments    = attachments,
					.width           = c->vk_swap_chain_extent.width,
					.height          = c->vk_swap_chain_extent.height,
					.layers          = 1,
				};

				PRODUCTION_ASSERT(vkCreateFramebuffer(c->vk_logical_device, &framebuffer_info, nullptr, &c->vk_swap_chain_framebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer!");
			}
		}

		static void vk_init_command_pools(RenderContext *c)
		{
			auto indices = vk_find_queue_families(c->vk_physical_device, c);

			VkCommandPoolCreateInfo pool_info{
				.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags            = 0,
				.queueFamilyIndex = indices.graphics_family.value(),
			};

			PRODUCTION_ASSERT(vkCreateCommandPool(c->vk_logical_device, &pool_info, nullptr, &c->vk_command_pool) == VK_SUCCESS, "Failed to create command pool!");
		}

		static void vk_init_command_buffers(RenderContext *c)
		{
			c->vk_command_buffers.resize(c->vk_swap_chain_framebuffers.size());

			VkCommandBufferAllocateInfo alloc_info{
				.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool        = c->vk_command_pool,
				.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = (u32)c->vk_command_buffers.size(),
			};

			PRODUCTION_ASSERT(vkAllocateCommandBuffers(c->vk_logical_device, &alloc_info, &c->vk_command_buffers[0]) == VK_SUCCESS, "Failed to allocate command buffers!");

			for (size_t i = 0; i < c->vk_command_buffers.size(); i++)
			{
				const auto &command_buffer = c->vk_command_buffers[i];
				VkCommandBufferBeginInfo begin_info{
					.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					.flags            = 0,
					.pInheritanceInfo = nullptr,
				};

				PRODUCTION_ASSERT(vkBeginCommandBuffer(command_buffer, &begin_info) == VK_SUCCESS, "Failed to begin recording command buffer!");

				VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
				VkRenderPassBeginInfo render_pass_info{
					.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass  = c->vk_render_pass,
					.framebuffer = c->vk_swap_chain_framebuffers[i],
					.renderArea =
						{
							.offset = {0, 0},
							.extent = c->vk_swap_chain_extent,
						},
					.clearValueCount = 1,
					.pClearValues    = &clear_color,
				};

				vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, c->vk_graphics_pipeline);
				vkCmdDraw(command_buffer, 3, 1, 0, 0);

				vkCmdEndRenderPass(command_buffer);

				PRODUCTION_ASSERT(vkEndCommandBuffer(command_buffer) == VK_SUCCESS, "Failed to record command buffer!");
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

			PRODUCTION_ASSERT(vkCreateInstance(&vk_instance_create_info, nullptr, &c->vk_instance) == VK_SUCCESS, "Failed to init vulkan.");
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
		}

		RenderContext *init_render_context(const Window *window, bool debug)
		{
			auto *c = static_cast<RenderContext *>(persist_alloc(sizeof(RenderContext)));
			new (c) RenderContext();
			PRODUCTION_ASSERT(c != nullptr, "Failed to allocate render context!");
			c->debug = debug;

			vk_init(window, c);
			return c;
		}

		void destroy_render_context(RenderContext *c)
		{
			vkDestroyCommandPool(c->vk_logical_device, c->vk_command_pool, nullptr);
			for (auto *framebuffer : c->vk_swap_chain_framebuffers)
			{
				vkDestroyFramebuffer(c->vk_logical_device, framebuffer, nullptr);
			}
			vkDestroyPipeline(c->vk_logical_device, c->vk_graphics_pipeline, nullptr);
			vkDestroyPipelineLayout(c->vk_logical_device, c->vk_pipeline_layout, nullptr);
			vkDestroyRenderPass(c->vk_logical_device, c->vk_render_pass, nullptr);
			for (auto *view : c->vk_swap_chain_image_views)
			{
				vkDestroyImageView(c->vk_logical_device, view, nullptr);
			}
			vkDestroySwapchainKHR(c->vk_logical_device, c->vk_swap_chain, nullptr);
			vkDestroyDevice(c->vk_logical_device, nullptr);
			vkDestroySurfaceKHR(c->vk_instance, c->vk_surface, nullptr);
			vk_destroy_debug_messenger(c);
			vkDestroyInstance(c->vk_instance, nullptr);
		}
	} // namespace Platform
} // namespace Vultr