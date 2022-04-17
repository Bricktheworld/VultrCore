#pragma once
#include <types/types.h>
#include <glm/glm.hpp>

namespace Vultr
{
	struct DirectionalLight
	{
		Vec4 ambient  = Vec4(1);
		Vec4 diffuse  = Vec4(1);
		f32 specular  = 1;
		f32 intensity = 3;
	};
} // namespace Vultr