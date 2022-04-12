#pragma once
#include <types/types.h>

namespace Vultr
{
	struct Camera
	{
		bool enabled          = true;

		f64 fov               = 45.0f;
		f64 znear             = 0.1f;
		f64 zfar              = 100.0f;
		f64 exposure          = 1.0f;
		f64 bloom_intensity   = 1.0f;
		f64 bloom_threshold   = 1.2f;
		u16 bloom_quality     = 10;

		bool gamma_correction = true;
	};

	inline Mat4 projection_matrix(const Camera &camera, f32 screen_width, f32 screen_height) { return glm::perspective(camera.fov, (f64)screen_width / (f64)screen_height, camera.znear, camera.zfar); }
} // namespace Vultr