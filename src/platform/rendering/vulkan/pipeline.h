#pragma once
#include "render_context.h"

namespace Vultr
{
	namespace Vulkan
	{
		struct GraphicsPipeline
		{
			VkPipelineLayout pipeline_layout = nullptr;
			VkPipeline graphics_pipeline     = nullptr;
		};

		GraphicsPipeline create_graphics_pipeline();
		void destroy_graphics_pipeline(GraphicsPipeline *pipeline);
	} // namespace Vulkan
} // namespace Vultr