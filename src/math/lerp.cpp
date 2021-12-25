#include <math/lerp.h>
#include <tgmath.h>

namespace Vultr::Math
{
	f32 lerp(f32 a, f32 b, f32 f) { return a + f * (b - a); }
	f32 lerp(f32 a, f32 b, f32 f, f32 dt) { return lerp(a, b, 1 - pow(f, dt)); }

	Vec2 lerp(Vec2 a, Vec2 b, f32 f, f32 dt)
	{
		f32 x = lerp(a.x, b.x, f, dt);
		f32 y = lerp(a.y, b.y, f, dt);
		return Vec2(x, y);
	}
} // namespace Vultr::Math
