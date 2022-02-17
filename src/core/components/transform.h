#pragma once
#include <types/types.h>
#include <glm/glm.hpp>

namespace Vultr
{
	struct Transform
	{
		Vec3 position = Vec3(0);
		Quat rotation = Quat(1, 0, 0, 0);
		Vec3 scale    = Vec3(1);
	};

	inline Vec3 forward(const Transform &transform) { return transform.rotation * Vec3(0, 0, -1); }
	inline Vec3 right(const Transform &transform) { return transform.rotation * Vec3(1, 0, 0); }
	inline Vec3 up(const Transform &transform) { return transform.rotation * Vec3(0, 1, 0); }
} // namespace Vultr