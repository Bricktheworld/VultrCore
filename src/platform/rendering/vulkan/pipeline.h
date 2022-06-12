#pragma once
#include "render_context.h"
#include <utils/traits.h>

namespace Vultr
{
	namespace Vulkan
	{
		struct GraphicsPipeline
		{
			Platform::GraphicsPipelineInfo layout{};
			VkPipelineLayout vk_layout = nullptr;
			VkPipeline vk_pipeline     = nullptr;
		};

		GraphicsPipeline *init_pipeline(Platform::RenderContext *c, Platform::Shader *shader, const Platform::GraphicsPipelineInfo &info);
		GraphicsPipeline *init_pipeline(Platform::RenderContext *c, Platform::Framebuffer *framebuffer, Platform::Shader *shader, const Platform::GraphicsPipelineInfo &info);
		void bind_pipeline(Platform::CmdBuffer *cmd, GraphicsPipeline *pipeline);
		void destroy_pipeline(Platform::RenderContext *c, GraphicsPipeline *pipeline);
	} // namespace Vulkan
} // namespace Vultr
