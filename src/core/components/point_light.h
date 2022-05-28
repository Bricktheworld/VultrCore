#pragma once
#include <ecs/component.h>
#include <types/types.h>
#include <platform/rendering.h>

namespace Vultr
{
	VCOMPONENT_BEGIN(PointLight)
	VCOMPONENT_FIELD(Vec4, color, Vec4(1))
	VCOMPONENT_FIELD(f32, strength, 1)
	VCOMPONENT_END()
} // namespace Vultr