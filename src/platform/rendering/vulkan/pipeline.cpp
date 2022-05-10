#include "pipeline.h"
#include "framebuffer.h"
#include "texture.h"

namespace Vultr
{
	namespace Platform
	{
		static VkFormat vk_format(VertexAttributeType type)
		{
			switch (type)
			{
				case VertexAttributeType::F32:
					return VK_FORMAT_R32_SFLOAT;
				case VertexAttributeType::F32_VEC2:
					return VK_FORMAT_R32G32_SFLOAT;
				case VertexAttributeType::F32_VEC3:
					return VK_FORMAT_R32G32B32_SFLOAT;
				case VertexAttributeType::F32_VEC4:
					return VK_FORMAT_R32G32B32A32_SFLOAT;
				case VertexAttributeType::S32:
					return VK_FORMAT_R32_SINT;
				case VertexAttributeType::S32_VEC2:
					return VK_FORMAT_R32G32_SINT;
				case VertexAttributeType::S32_VEC3:
					return VK_FORMAT_R32G32B32_SINT;
				case VertexAttributeType::S32_VEC4:
					return VK_FORMAT_R32G32B32A32_SINT;
				case VertexAttributeType::U32:
					return VK_FORMAT_R32_UINT;
				case VertexAttributeType::U32_VEC2:
					return VK_FORMAT_R32G32_UINT;
				case VertexAttributeType::U32_VEC3:
					return VK_FORMAT_R32G32B32_UINT;
				case VertexAttributeType::U32_VEC4:
					return VK_FORMAT_R32G32B32A32_UINT;
				case VertexAttributeType::F64:
					return VK_FORMAT_R64_SFLOAT;
				case VertexAttributeType::F64_VEC2:
					return VK_FORMAT_R64G64_SFLOAT;
				case VertexAttributeType::F64_VEC3:
					return VK_FORMAT_R64G64B64_SFLOAT;
				case VertexAttributeType::F64_VEC4:
					return VK_FORMAT_R64G64B64A64_SFLOAT;
				default:
					THROW("Invalid vertex attribute type!");
			}
		}

		static GraphicsPipeline *init_vk_pipeline(RenderContext *c, const GraphicsPipelineInfo &info, VkRenderPass render_pass, bool has_depth)
		{
			using namespace Vulkan;

			auto *d                                  = Vulkan::get_device(c);
			auto *sc                                 = Vulkan::get_swapchain(c);
			auto *pipeline                           = v_alloc<GraphicsPipeline>();
			pipeline->layout                         = info;

			VkPipelineShaderStageCreateInfo stages[] = {{
															.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
															.stage  = VK_SHADER_STAGE_VERTEX_BIT,
															.module = info.shader->vert_module,
															.pName  = "main",
														},
														{
															.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
															.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
															.module = info.shader->frag_module,
															.pName  = "main",
														}};
			VkVertexInputBindingDescription binding_description{
				.binding   = 0,
				.stride    = info.description.stride,
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
			};

			VkVertexInputAttributeDescription attribute_descriptions[info.description.attribute_descriptions.size()];
			u32 i = 0;
			for (auto &description : info.description.attribute_descriptions)
			{
				attribute_descriptions[i].location = i;
				attribute_descriptions[i].binding  = 0;
				attribute_descriptions[i].format   = vk_format(description.type);
				attribute_descriptions[i].offset   = description.offset;
				i++;
			}

			VkPipelineVertexInputStateCreateInfo vertex_input_info{
				.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.vertexBindingDescriptionCount   = 1,
				.pVertexBindingDescriptions      = &binding_description,
				.vertexAttributeDescriptionCount = static_cast<u32>(info.description.attribute_descriptions.size()),
				.pVertexAttributeDescriptions    = &attribute_descriptions[0],
			};

			VkPipelineInputAssemblyStateCreateInfo input_assembly{
				.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.primitiveRestartEnable = VK_FALSE,
			};

			VkPipelineViewportStateCreateInfo viewport_state{
				.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.viewportCount = 1,
				.pViewports    = nullptr,
				.scissorCount  = 1,
				.pScissors     = nullptr,
			};

			VkPipelineRasterizationStateCreateInfo rasterizer{
				.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				.depthClampEnable        = VK_FALSE,
				.rasterizerDiscardEnable = VK_FALSE,
				.polygonMode             = VK_POLYGON_MODE_FILL,
				.cullMode                = VK_CULL_MODE_BACK_BIT,
				.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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

			VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

			VkPipelineDynamicStateCreateInfo dynamic_state{
				.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.dynamicStateCount = 2,
				.pDynamicStates    = dynamic_states,
			};

			VkPushConstantRange push_constant_range{
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				.offset     = 0,
				.size       = sizeof(PushConstant),
			};

			VkDescriptorSetLayout layouts[2] = {sc->default_descriptor_set_layout, pipeline->layout.shader->vk_custom_layout};

			VkPipelineLayoutCreateInfo pipeline_layout_info{
				.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount         = 2,
				.pSetLayouts            = layouts,
				.pushConstantRangeCount = 1,
				.pPushConstantRanges    = &push_constant_range,
			};

			VK_CHECK(vkCreatePipelineLayout(d->device, &pipeline_layout_info, nullptr, &pipeline->vk_layout));

			VkPipelineDepthStencilStateCreateInfo depth_stencil_info{
				.sType             = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
				.depthTestEnable   = VK_TRUE,
				.depthWriteEnable  = VK_TRUE,
				.depthCompareOp    = VK_COMPARE_OP_LESS,
				.stencilTestEnable = VK_FALSE,
			};

			VkGraphicsPipelineCreateInfo pipeline_info{
				.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.stageCount          = 2,
				.pStages             = stages,
				.pVertexInputState   = &vertex_input_info,
				.pInputAssemblyState = &input_assembly,
				.pViewportState      = &viewport_state,
				.pRasterizationState = &rasterizer,
				.pMultisampleState   = &multisampling,
				.pDepthStencilState  = has_depth ? &depth_stencil_info : nullptr,
				.pColorBlendState    = &color_blending,
				.pDynamicState       = &dynamic_state,
				.layout              = pipeline->vk_layout,
				.renderPass          = render_pass,
				.subpass             = 0,
				.basePipelineHandle  = VK_NULL_HANDLE,
				.basePipelineIndex   = -1,
			};

			VK_CHECK(vkCreateGraphicsPipelines(d->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline->vk_pipeline));
			return pipeline;
		}

		GraphicsPipeline *init_pipeline(RenderContext *c, const GraphicsPipelineInfo &info) { return init_vk_pipeline(c, info, Vulkan::get_swapchain(c)->render_pass, false); }

		GraphicsPipeline *init_pipeline(RenderContext *c, Framebuffer *framebuffer, const GraphicsPipelineInfo &info) { return init_vk_pipeline(c, info, framebuffer->vk_renderpass, Vulkan::has_depth(framebuffer)); }

		void destroy_pipeline(RenderContext *c, GraphicsPipeline *pipeline)
		{
			auto *d = Vulkan::get_device(c);
			Vulkan::wait_resource_not_in_use(Vulkan::get_swapchain(c), pipeline);
			vkDestroyPipelineLayout(d->device, pipeline->vk_layout, nullptr);
			vkDestroyPipeline(d->device, pipeline->vk_pipeline, nullptr);
			pipeline->vk_layout = nullptr;
			pipeline->vk_layout = nullptr;
			v_free(pipeline);
		}

		static Platform::Texture *get_placeholder_texture(RenderContext *c, SamplerType type)
		{
			switch (type)
			{
				case SamplerType::NORMAL:
					return c->normal_texture;
				case SamplerType::ALBEDO:
				case SamplerType::METALLIC:
				case SamplerType::ROUGHNESS:
				case SamplerType::AMBIENT_OCCLUSION:
					return c->white_texture;
			}
		}

		void bind_pipeline(CmdBuffer *cmd, GraphicsPipeline *pipeline)
		{
			ASSERT(pipeline != nullptr && pipeline->vk_pipeline != nullptr, "Cannot bind nullptr pipeline!");

			Vector<VkWriteDescriptorSet> write_sets{};
			Vector<VkDescriptorBufferInfo> ubo_info{};
			Vector<VkDescriptorImageInfo> tex_info{};

			auto *shader = pipeline->layout.shader;
			auto *c = cmd->render_context;
			auto *d = Vulkan::get_device(c);
			Platform::Lock lock(shader->mutex);

			u32 ubo_info_len   = 0;
			u32 tex_info_len   = 0;
			u32 write_sets_len = 0;
			for (auto &set : shader->allocated_descriptor_sets)
			{
				if (!set->updated.at(cmd->image_index))
					continue;
				ubo_info_len++;
				write_sets_len++;
				for (auto &sampler : set->sampler_bindings)
				{
					tex_info_len++;
					write_sets_len++;
				}
			}

			write_sets.resize(write_sets_len);
			ubo_info.resize(ubo_info_len);
			tex_info.resize(tex_info_len);

			u32 ubo_info_index  = 0;
			u32 tex_info_index  = 0;
			u32 write_set_index = 0;
			for (auto &set : shader->allocated_descriptor_sets)
			{
				if (!set->updated.at(cmd->image_index))
					continue;

				set->updated.set(cmd->image_index, false);
				auto *vk_set = set->vk_frame_descriptor_sets[cmd->image_index];

				ubo_info[ubo_info_index] = {
					.buffer = set->uniform_buffer_binding.buffer.buffer,
					.offset = 0,
					.range  = pad_size(d, set->shader->reflection.uniform_size),
				};
				write_sets[write_set_index] = {
					.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext           = nullptr,
					.dstSet          = vk_set,
					.dstBinding      = 0,
					.descriptorCount = 1,
					.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pBufferInfo     = &ubo_info[ubo_info_index],
				};
				write_set_index++;
				ubo_info_index++;

				u32 i = 1;
				for (auto &sampler : set->sampler_bindings)
				{
					auto type                  = shader->reflection.samplers[i - 1].type;
					Platform::Texture *texture = get_placeholder_texture(c, type);

					if (sampler.has_value())
					{
						if (!sampler.value().loaded<Platform::Texture *>())
							continue;
						texture = sampler.value().value<Platform::Texture *>();
					}

					printf("Updating descriptor set with texture %p sampler %p and image_view %p\n", texture, texture->sampler, texture->image_view);
					tex_info[tex_info_index] = {
						.sampler     = texture->sampler,
						.imageView   = texture->image_view,
						.imageLayout = texture->layout,
					};

					write_sets[write_set_index] = {
						.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.pNext           = nullptr,
						.dstSet          = vk_set,
						.dstBinding      = i,
						.descriptorCount = 1,
						.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						.pImageInfo      = &tex_info[tex_info_index],
					};
					write_set_index++;
					tex_info_index++;
					i++;
				}
			}

			vkUpdateDescriptorSets(d->device, write_sets_len, write_sets_len == 0 ? nullptr : &write_sets[0], 0, nullptr);

			vkCmdBindPipeline(cmd->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_pipeline);
			vkCmdBindDescriptorSets(cmd->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_layout, 0, 1, &cmd->frame->default_uniform_descriptor, 0, nullptr);
			Vulkan::depend_resource(cmd, shader);
			Vulkan::depend_resource(cmd, pipeline);
		}
	} // namespace Platform

} // namespace Vultr
