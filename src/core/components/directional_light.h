#pragma once
#include <ecs/component.h>
#include <types/types.h>
#include <glm/glm.hpp>

namespace Vultr
{
	VCOMPONENT_BEGIN(DirectionalLight)
	VCOMPONENT_FIELD(Vec4, ambient, Vec4(1))
	VCOMPONENT_FIELD(Vec4, diffuse, Vec4(1))
	VCOMPONENT_FIELD(f32, specular, 1)
	VCOMPONENT_FIELD(f32, intensity, 3)
	VCOMPONENT_END()
} // namespace Vultr