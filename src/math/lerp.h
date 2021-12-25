#pragma once
#include <types/types.h>
#include <glm/glm.hpp>

namespace Vultr::Math
{
	f32 lerp(f32 a, f32 b, f32 f);
	f32 lerp(f32 a, f32 b, f32 f, f32 dt);

	Vec2 lerp(Vec2 a, Vec2 b, f32 f, f32 dt);
} // namespace Vultr::Math
