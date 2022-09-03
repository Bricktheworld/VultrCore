#pragma once
#include <ecs/component.h>
#include <types/types.h>

namespace Vultr
{
	VCOMPONENT_BEGIN(Camera)
	VCOMPONENT_FIELD(bool, enabled, true)
	VCOMPONENT_FIELD(f64, fov, 45.0f)
	VCOMPONENT_FIELD(f64, znear, 0.1f)
	VCOMPONENT_FIELD(f64, zfar, 100.0f)
	VCOMPONENT_FIELD(f64, exposure, 1.0f)
	VCOMPONENT_FIELD(f64, bloom_intensity, 1.0f)
	VCOMPONENT_FIELD(f64, bloom_threshold, 1.2f)
	VCOMPONENT_FIELD(u16, bloom_quality, 10)
	VCOMPONENT_FIELD(bool, gamma_correction, true)
	VCOMPONENT_END()

	inline Mat4 projection_matrix(const Camera &camera, f32 screen_width, f32 screen_height) { return glm::perspective(camera.fov, (f64)screen_width / (f64)screen_height, camera.znear, camera.zfar); }
} // namespace Vultr