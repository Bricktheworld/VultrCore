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
	} // namespace Vulkan
} // namespace Vultr