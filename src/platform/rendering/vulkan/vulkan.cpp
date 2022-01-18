#include "device.cpp"
#include "render_context.cpp"
#include "vulkan_buffer.cpp"
#if 0
#include "../../platform.h"
#include <types/optional.h>
#include <types/array.h>

namespace Vultr
{
	namespace Platform
	{
		struct Vertex
		{
			Vec2 position;
			Vec3 color;
		};

		static VkVertexInputBindingDescription get_binding_description()
		{
			return {
				.binding   = 0,
				.stride    = sizeof(Vertex),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
			};
		}

		static Array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions()
		{
			Array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};
			attribute_descriptions[0] = {
				.location = 0,
				.binding  = 0,
				.format   = VK_FORMAT_R32G32_SFLOAT,
				.offset   = offsetof(Vertex, position),
			};
			attribute_descriptions[1] = {
				.location = 1,
				.binding  = 0,
				.format   = VK_FORMAT_R32G32B32_SFLOAT,
				.offset   = offsetof(Vertex, color),
			};

			return attribute_descriptions;
		}


		static VkShaderModule init_shader_module(RenderContext *c, const Buffer &src)
		{
			VkShaderModuleCreateInfo create_info{};
			create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			create_info.codeSize = src.size();
			create_info.pCode    = reinterpret_cast<const u32 *>(src.storage);
			VkShaderModule shader_module;
			ASSERT(vkCreateShaderModule(c->device, &create_info, nullptr, &shader_module) == VK_SUCCESS, "Failed to init vulkan shader!");
			return shader_module;
		}

		static void init_render_pass(RenderContext *c)
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
		static void init_graphics_pipeline(RenderContext *c)
		{
			Buffer vert_shader_src;
			fread_all(Path("./build/shaders/basic_vert.spv"), &vert_shader_src);

			Buffer frag_shader_src;
			fread_all(Path("./build/shaders/basic_frag.spv"), &frag_shader_src);

			auto vert_shader_module                  = init_shader_module(c, vert_shader_src);
			auto frag_shader_module                  = init_shader_module(c, frag_shader_src);

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

			auto binding_description                 = get_binding_description();
			auto attribute_descriptions              = get_attribute_descriptions();
			VkPipelineVertexInputStateCreateInfo vertex_input_info{
				.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.vertexBindingDescriptionCount   = 1,
				.pVertexBindingDescriptions      = &binding_description,
				.vertexAttributeDescriptionCount = static_cast<u32>(attribute_descriptions.size()),
				.pVertexAttributeDescriptions    = &attribute_descriptions[0],
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

		static void init_framebuffers(RenderContext *c)
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

		static u32 find_memory_type(RenderContext *c, u32 type_filter, VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties mem_properties;
			vkGetPhysicalDeviceMemoryProperties(c->physical_device, &mem_properties);

			for (u32 i = 0; i < mem_properties.memoryTypeCount; i++)
			{
				if ((type_filter & (1 << i)) && ((mem_properties.memoryTypes[i].propertyFlags & properties) == properties))
				{
					return i;
				}
			}
		}

		static void init_buffer(RenderContext *c, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharing_mode, VkBuffer *buffer, VkDeviceMemory *buffer_memory)
		{
			VkBufferCreateInfo buffer_info{
				.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size        = size,
				.usage       = usage,
				.sharingMode = sharing_mode,
			};

			PRODUCTION_ASSERT(vkCreateBuffer(c->device, &buffer_info, nullptr, buffer) == VK_SUCCESS, "Failed to create vulkan buffer!");

			VkMemoryRequirements mem_requirements;
			vkGetBufferMemoryRequirements(c->device, *buffer, &mem_requirements);

			VkMemoryAllocateInfo alloc_info{
				.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize  = mem_requirements.size,
				.memoryTypeIndex = find_memory_type(c, mem_requirements.memoryTypeBits, properties),
			};

			PRODUCTION_ASSERT(vkAllocateMemory(c->device, &alloc_info, nullptr, buffer_memory) == VK_SUCCESS, "Failed to allocate memory for vertex buffer!");
			vkBindBufferMemory(c->device, *buffer, *buffer_memory, 0);
		}

		static void copy_buffer(RenderContext *c, VkBuffer dst_buffer, VkBuffer src_buffer, VkDeviceSize size)
		{
			VkCommandBufferAllocateInfo alloc_info{
				.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool        = c->command_pool,
				.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};

			VkCommandBuffer command_buffer;
			vkAllocateCommandBuffers(c->device, &alloc_info, &command_buffer);

			VkCommandBufferBeginInfo begin_info{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			};
			vkBeginCommandBuffer(command_buffer, &begin_info);

			VkBufferCopy copy_region{.size = size};
			vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);
			vkEndCommandBuffer(command_buffer);

			VkSubmitInfo submitInfo{};
			submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers    = &command_buffer;

			vkQueueSubmit(c->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(c->graphics_queue);
			vkFreeCommandBuffers(c->device, c->command_pool, 1, &command_buffer);
		}

		static void init_vertex_buffer(RenderContext *c)
		{
			auto buffer_size = sizeof(c->vertices[0]) * c->vertices.size();

			VkBuffer staging_buffer{};
			VkDeviceMemory staging_buffer_memory{};
			init_buffer(c, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_SHARING_MODE_EXCLUSIVE, &staging_buffer,
						&staging_buffer_memory);

			void *data;
			vkMapMemory(c->device, staging_buffer_memory, 0, buffer_size, 0, &data);
			memcpy(data, &c->vertices[0], reinterpret_cast<size_t>(buffer_size));
			vkUnmapMemory(c->device, staging_buffer_memory);

			init_buffer(c, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE, &c->vertex_buffer,
						&c->vertex_buffer_memory);
			copy_buffer(c, c->vertex_buffer, staging_buffer, buffer_size);

			vkDestroyBuffer(c->device, staging_buffer, nullptr);
			vkFreeMemory(c->device, staging_buffer_memory, nullptr);
		}

		static void init_index_buffer(RenderContext *c)
		{
			VkDeviceSize buffer_size = sizeof(c->indices[0]) * c->indices.size();

			VkBuffer staging_buffer;
			VkDeviceMemory staging_buffer_memory;
			init_buffer(c, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_SHARING_MODE_EXCLUSIVE, &staging_buffer,
						&staging_buffer_memory);

			void *data;
			vkMapMemory(c->device, staging_buffer_memory, 0, buffer_size, 0, &data);
			memcpy(data, &c->indices[0], reinterpret_cast<size_t>(buffer_size));
			vkUnmapMemory(c->device, staging_buffer_memory);

			init_buffer(c, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE, &c->index_buffer,
						&c->index_buffer_memory);

			copy_buffer(c, c->index_buffer, staging_buffer, buffer_size);

			vkDestroyBuffer(c->device, staging_buffer, nullptr);
			vkFreeMemory(c->device, staging_buffer_memory, nullptr);
		}


		// TODO(Brandon): Don't do this here.
		static void init_command_buffers(RenderContext *c)
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

				VkBuffer vertex_buffers[] = {c->vertex_buffer};
				VkDeviceSize offsets[]    = {0};
				vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
				vkCmdBindIndexBuffer(command_buffer, c->index_buffer, 0, VK_INDEX_TYPE_UINT16);

				vkCmdDrawIndexed(command_buffer, static_cast<u32>(c->indices.size()), 1, 0, 0, 0);

				vkCmdEndRenderPass(command_buffer);

				PRODUCTION_ASSERT(vkEndCommandBuffer(command_buffer) == VK_SUCCESS, "Failed to record command buffer!");
			}
		}

		static void init_concurrency(RenderContext *c)
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

		static void recreate_swapchain(const Window *window, RenderContext *c)
		{
			recreate_swapchain(&c->device, window);
			init_render_pass(c);
			init_graphics_pipeline(c);
			init_framebuffers(c);
			init_command_buffers(c);
		}


		void draw_frame(const Window *window, RenderContext *c)
		{
			vkWaitForFences(c->device, 1, &c->in_flight_fences[c->current_frame], VK_TRUE, UINT64_MAX);

			u32 image_index;
			auto res = vkAcquireNextImageKHR(c->device, c->swap_chain, U64Max, c->image_available_semaphores[c->current_frame], VK_NULL_HANDLE, &image_index);
			if (res == VK_ERROR_OUT_OF_DATE_KHR)
			{
				// If this occurs, then try again next draw frame call.
				recreate_swapchain(window, c);
				return;
			}
			else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
			{
				THROW("Failed to acquire swap chain images!");
			}

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
			res = vkQueuePresentKHR(c->present_queue, &present_info);
			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || c->framebuffer_was_resized)
			{
				c->framebuffer_was_resized = false;
				recreate_swapchain(window, c);
			}
			else if (res != VK_SUCCESS)
			{
				THROW("Failed to present swap chain images!");
			}

			c->current_frame = (c->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
		}


} // namespace Platform
} // namespace Vultr
#endif
