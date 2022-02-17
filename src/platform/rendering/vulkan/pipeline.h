#pragma once
#include "render_context.h"
#include "shader.h"
#include "descriptor_set.h"

namespace Vultr
{
	namespace Platform
	{
		struct GraphicsPipeline
		{
			Platform::GraphicsPipelineInfo layout{};
			VkPipelineLayout vk_layout = nullptr;
			VkPipeline vk_pipeline     = nullptr;
		};
	} // namespace Platform
} // namespace Vultr
