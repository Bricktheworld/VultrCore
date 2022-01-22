#pragma once
#include <types/types.h>
#include <glm/glm.hpp>

namespace Vultr
{
	struct Transform
	{
		Vec3 position{};
		Quat rotation{};
		Vec3 scale{};
	};
} // namespace Vultr