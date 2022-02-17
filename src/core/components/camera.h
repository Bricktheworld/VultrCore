#pragma once
#include <types/types.h>

namespace Vultr
{
	struct Camera
	{
		bool enabled          = true;

		f32 fov               = 45.0f;
		f32 znear             = 0.1f;
		f32 zfar              = 100.0f;
		f32 exposure          = 1.0f;
		f32 bloom_intensity   = 1.0f;
		f32 bloom_threshold   = 1.2f;
		u16 bloom_quality     = 10;

		bool gamma_correction = true;
	};
} // namespace Vultr