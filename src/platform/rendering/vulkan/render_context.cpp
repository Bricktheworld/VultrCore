#include "render_context.h"
#include "vulkan_buffer.h"

namespace Vultr
{
	namespace Vulkan
	{
		struct UniformBufferObject
		{
			Mat4 model;
			Mat4 view;
			Mat4 proj;
		};

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

		static VkShaderModule init_shader_module(Platform::RenderContext *c, const Buffer &src)
		{
			auto *d = &c->swap_chain.device;
			VkShaderModuleCreateInfo create_info{};
			create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			create_info.codeSize = src.size();
			create_info.pCode    = reinterpret_cast<const u32 *>(src.storage);
			VkShaderModule shader_module;
			ASSERT(vkCreateShaderModule(d->device, &create_info, nullptr, &shader_module) == VK_SUCCESS, "Failed to init vulkan shader!");
			return shader_module;
		}

		//		static void init_descriptor_set_layout(Platform::RenderContext *c)
		//		{
		//			auto *d = &c->swap_chain.device;
		//			VkDescriptorSetLayoutBinding ubo_layout_binding{
		//				.binding            = 0,
		//				.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		//				.descriptorCount    = 1,
		//				.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
		//				.pImmutableSamplers = nullptr,
		//			};
		//			VkDescriptorSetLayoutCreateInfo layout_info{
		//				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		//				.bindingCount = 1,
		//				.pBindings    = &ubo_layout_binding,
		//			};
		//			VK_CHECK(vkCreateDescriptorSetLayout(d->device, &layout_info, nullptr, &c->descriptor_set_layout));
		//		}

		// TODO(Brandon): Definitely don't do this here.
		static void init_graphics_pipeline(Platform::RenderContext *c)
		{
			auto *d = &c->swap_chain.device;
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

			auto *extent = &c->swap_chain.extent;

			VkViewport viewport{
				.x        = 0.0f,
				.y        = 0.0f,
				.width    = static_cast<f32>(extent->width),
				.height   = static_cast<f32>(extent->height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f,
			};

			VkRect2D scissor{
				.offset = {0, 0},
				.extent = *extent,
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

			VK_CHECK(vkCreatePipelineLayout(d->device, &pipeline_layout_info, nullptr, &c->pipeline_layout));

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
				.renderPass          = c->swap_chain.render_pass,
				.subpass             = 0,
				.basePipelineHandle  = VK_NULL_HANDLE,
				.basePipelineIndex   = -1,
			};

			VK_CHECK(vkCreateGraphicsPipelines(d->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c->graphics_pipeline));

			vkDestroyShaderModule(d->device, frag_shader_module, nullptr);
			vkDestroyShaderModule(d->device, vert_shader_module, nullptr);
		}

		static void init_vertex_buffer(Platform::RenderContext *c)
		{
			auto *d             = &c->swap_chain.device;
			auto buffer_size    = sizeof(c->vertices[0]) * c->vertices.size();

			auto staging_buffer = alloc_buffer(d, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_SHARING_MODE_EXCLUSIVE);

			void *data;
			vkMapMemory(d->device, staging_buffer.memory, 0, buffer_size, 0, &data);
			memcpy(data, &c->vertices[0], reinterpret_cast<size_t>(buffer_size));
			vkUnmapMemory(d->device, staging_buffer.memory);

			c->vertex_buffer = alloc_buffer(d, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE);
			copy_buffer(&c->swap_chain, c->vertex_buffer, staging_buffer, buffer_size);
			free_buffer(d, &staging_buffer);
		}

		static void init_index_buffer(Platform::RenderContext *c)
		{
			auto *d             = &c->swap_chain.device;

			auto buffer_size    = sizeof(c->indices[0]) * c->indices.size();

			auto staging_buffer = alloc_buffer(d, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_SHARING_MODE_EXCLUSIVE);

			void *data;
			vkMapMemory(d->device, staging_buffer.memory, 0, buffer_size, 0, &data);
			memcpy(data, &c->indices[0], reinterpret_cast<size_t>(buffer_size));
			vkUnmapMemory(d->device, staging_buffer.memory);

			c->index_buffer = alloc_buffer(d, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE);

			copy_buffer(&c->swap_chain, c->index_buffer, staging_buffer, buffer_size);

			free_buffer(d, &staging_buffer);
		}

		// TODO(Brandon): Don't do this here.
		static void init_command_buffers(Platform::RenderContext *c)
		{
			auto *d = &c->swap_chain.device;
			c->command_buffers.resize(c->swap_chain.framebuffers.size());

			VkCommandBufferAllocateInfo alloc_info{
				.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool        = c->swap_chain.graphics_command_pool,
				.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = static_cast<u32>(c->command_buffers.size()),
			};

			VK_CHECK(vkAllocateCommandBuffers(d->device, &alloc_info, &c->command_buffers[0]));

			for (size_t i = 0; i < c->command_buffers.size(); i++)
			{
				const auto &command_buffer = c->command_buffers[i];
				VkCommandBufferBeginInfo begin_info{
					.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					.flags            = 0,
					.pInheritanceInfo = nullptr,
				};

				VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

				VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
				VkRenderPassBeginInfo render_pass_info{
					.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass  = c->swap_chain.render_pass,
					.framebuffer = c->swap_chain.framebuffers[i],
					.renderArea =
						{
							.offset = {0, 0},
							.extent = c->swap_chain.extent,
						},
					.clearValueCount = 1,
					.pClearValues    = &clear_color,
				};

				vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, c->graphics_pipeline);

				VkBuffer vertex_buffers[] = {c->vertex_buffer.buffer};
				VkDeviceSize offsets[]    = {0};
				vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
				vkCmdBindIndexBuffer(command_buffer, c->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);

				vkCmdDrawIndexed(command_buffer, static_cast<u32>(c->indices.size()), 1, 0, 0, 0);

				vkCmdEndRenderPass(command_buffer);

				VK_CHECK(vkEndCommandBuffer(command_buffer))
			}
		}
		//		static void update_uniform_buffer(Platform::RenderContext *c, u32 image_index, f64 dt)
		//		{
		//			auto *sc = &c->swap_chain;
		//			auto *d  = &sc->device;
		//			c->rotation += 1 * dt;
		//
		//			auto extent = sc->extent;
		//
		//			UniformBufferObject ubo{
		//				.model = glm::rotate(glm::mat4(1.0f), glm::radians((f32)c->rotation), glm::vec3(0.0f, 0.0f, 1.0f)),
		//				.view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		//				.proj  = glm::perspective(glm::radians(45.0f), (f32)extent.width / (f32)extent.height, 0.1f, 10.0f),
		//			};
		//			ubo.proj[1][1] *= -1;
		//
		//			void *data;
		//			vkMapMemory(d->device, sc->uniform_buffers[image_index].memory, 0, sizeof(ubo), 0, &data);
		//			memcpy(data, &ubo, sizeof(ubo));
		//			vkUnmapMemory(d->device, sc->uniform_buffers[image_index].memory);
		//		}

	} // namespace Vulkan
	namespace Platform
	{
		RenderContext *init_render_context(const Platform::Window *window, bool debug)
		{
			auto *c = static_cast<RenderContext *>(persist_alloc(sizeof(RenderContext)));
			PRODUCTION_ASSERT(c != nullptr, "Failed to allocate render context!");
			new (c) RenderContext();

			auto device   = Vulkan::init_device(window, debug, Vulkan::debug_cb);
			c->swap_chain = Vulkan::init_swapchain(device, window);
			//			Vulkan::init_descriptor_set_layout(c);
			Vulkan::init_graphics_pipeline(c);
			Vulkan::init_vertex_buffer(c);
			Vulkan::init_index_buffer(c);
			//			Vulkan::init_command_buffers(c);
			return c;
		}

		void draw_frame(const Platform::Window *window, RenderContext *c, f64 dt)
		{
			if check (Vulkan::acquire_swapchain(&c->swap_chain), u32 image_index, auto _)
			{
				//				Vulkan::update_uniform_buffer(c, image_index, dt);
				Vulkan::submit_swapchain(&c->swap_chain, image_index, c->command_buffers.size(), &c->command_buffers[0]);
			}
			else
			{
				auto *d = &c->swap_chain.device;
				Vulkan::recreate_swapchain(&c->swap_chain, window);

				vkDestroyPipeline(d->device, c->graphics_pipeline, nullptr);
				vkDestroyPipelineLayout(d->device, c->pipeline_layout, nullptr);

				Vulkan::init_graphics_pipeline(c);
				//				Vulkan::init_command_buffers(c);
			}
		}

		void destroy_render_context(RenderContext *c)
		{
			auto *d = &c->swap_chain.device;
			// Make sure don't destroy anything that is in use.
			wait_idle(d);

			Vulkan::free_buffer(d, &c->vertex_buffer);
			Vulkan::free_buffer(d, &c->index_buffer);

			//			vkDestroyDescriptorSetLayout(d->device, c->descriptor_set_layout, nullptr);

			vkDestroyPipeline(d->device, c->graphics_pipeline, nullptr);
			vkDestroyPipelineLayout(d->device, c->pipeline_layout, nullptr);

			destroy_swapchain(&c->swap_chain);
		}
	} // namespace Platform
} // namespace Vultr