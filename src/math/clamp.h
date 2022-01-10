#pragma once
#include <types/types.h>

namespace Vultr
{
	// Copy and pasted but whatever, it's much easier to read.
	inline constexpr f32 clamp(f32 x, f32 min, f32 max)
	{
		const auto t = x < min ? min : x;
		return t > max ? max : t;
	}
	inline constexpr f64 clamp(f64 x, f64 min, f64 max)
	{
		const auto t = x < min ? min : x;
		return t > max ? max : t;
	}
	inline constexpr s32 clamp(s32 x, s32 min, s32 max)
	{
		const auto t = x < min ? min : x;
		return t > max ? max : t;
	}
	inline constexpr s64 clamp(s64 x, s64 min, s64 max)
	{
		const auto t = x < min ? min : x;
		return t > max ? max : t;
	}
	inline constexpr u32 clamp(u32 x, u32 min, u32 max)
	{
		const auto t = x < min ? min : x;
		return t > max ? max : t;
	}
	inline constexpr u64 clamp(u64 x, u64 min, u64 max)
	{
		const auto t = x < min ? min : x;
		return t > max ? max : t;
	}
} // namespace Vultr