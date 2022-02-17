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

	inline Vec3 forward(Transform *transform) { return transform->rotation * Vec3(0, 0, -1); }
	inline Vec3 right(Transform *transform) { return transform->rotation * Vec3(1, 0, 0); }
	inline Vec3 up(Transform *transform) { return transform->rotation * Vec3(0, 1, 0); }
} // namespace Vultr