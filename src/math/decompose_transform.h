#pragma once
#include <glm/glm.hpp>

namespace Vultr::Math
{
	bool decompose_transform(const Mat4 &transform, Vec3 &translation, Vec3 &rotation, Vec3 &scale);
} // namespace Vultr::Math
