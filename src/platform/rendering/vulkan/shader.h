#pragma once
#include <vulkan/vulkan.h>
#include "device.h"
#include <filesystem/filesystem.h>
#include <platform/rendering.h>

namespace Vultr
{
	namespace Platform
	{
		struct Shader
		{
			VkShaderModule vert_module = nullptr;
			VkShaderModule frag_module = nullptr;
		};
	} // namespace Platform
} // namespace Vultr
