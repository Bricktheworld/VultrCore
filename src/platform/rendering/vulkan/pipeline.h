#pragma once
#include "render_context.h"
#include "shader.h"

namespace Vultr
{
	namespace Platform
	{
		struct GraphicsPipeline
		{
			VkPipelineLayout vk_layout = nullptr;
			VkPipeline vk_pipeline     = nullptr;
			Shader *vert               = nullptr;
			Shader *frag               = nullptr;
		};
	} // namespace Platform
} // namespace Vultr
