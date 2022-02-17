#include "pipeline.h"

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

		GraphicsPipeline *init_pipeline(RenderContext *c, const GraphicsPipelineInfo &info)
		{
			using namespace Vulkan;
			ASSERT(info.frag->type == ShaderType::FRAG && info.vert->type == ShaderType::VERT, "Incorrect shaders provided");

			auto *sc                                 = Vulkan::get_swapchain(c);
			auto *d                                  = Vulkan::get_device(c);
			auto *pipeline                           = v_alloc<GraphicsPipeline>();
			pipeline->layout                         = info;

			VkPipelineShaderStageCreateInfo stages[] = {{
															.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
															.stage  = VK_SHADER_STAGE_VERTEX_BIT,
															.module = info.vert->vk_module,
															.pName  = "main",
														},
														{
															.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
															.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
															.module = info.frag->vk_module,
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

			VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

			VkPipelineDynamicStateCreateInfo dynamic_state{
				.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.dynamicStateCount = 2,
				.pDynamicStates    = dynamic_states,
			};

			VkPushConstantRange push_constant_range{
				.offset     = 0,
				.size       = sizeof(PushConstant),
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			};

			VkDescriptorSetLayout layouts[pipeline->layout.descriptor_layouts.size()];

			u32 j = 0;
			for (auto *descriptor_layout : pipeline->layout.descriptor_layouts)
			{
				layouts[j] = descriptor_layout->vk_layout;
				j++;
			}

			VkPipelineLayoutCreateInfo pipeline_layout_info{
				.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount         = static_cast<u32>(pipeline->layout.descriptor_layouts.size()),
				.pSetLayouts            = layouts,
				.pushConstantRangeCount = 1,
				.pPushConstantRanges    = &push_constant_range,
			};

			VK_CHECK(vkCreatePipelineLayout(d->device, &pipeline_layout_info, nullptr, &pipeline->vk_layout));

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
				.pDynamicState       = &dynamic_state,
				.layout              = pipeline->vk_layout,
				.renderPass          = sc->render_pass,
				.subpass             = 0,
				.basePipelineHandle  = VK_NULL_HANDLE,
				.basePipelineIndex   = -1,
			};

			VK_CHECK(vkCreateGraphicsPipelines(d->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline->vk_pipeline));
			return pipeline;
		}

		void destroy_pipeline(RenderContext *c, GraphicsPipeline *pipeline)
		{
			auto *d = Vulkan::get_device(c);
			vkDestroyPipelineLayout(d->device, pipeline->vk_layout, nullptr);
			vkDestroyPipeline(d->device, pipeline->vk_pipeline, nullptr);
			pipeline->vk_layout = nullptr;
			pipeline->vk_layout = nullptr;
			v_free(pipeline);
		}

		void bind_pipeline(CmdBuffer *cmd, GraphicsPipeline *pipeline)
		{
			ASSERT(pipeline != nullptr && pipeline->vk_pipeline != nullptr, "Cannot bind nullptr pipeline!");
			vkCmdBindPipeline(cmd->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_pipeline);
		}
	} // namespace Platform
} // namespace Vultr
