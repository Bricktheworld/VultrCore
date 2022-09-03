#pragma once
#include <ecs/component.h>
#include <types/types.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace Vultr
{
	VCOMPONENT_BEGIN(Transform)
	VCOMPONENT_FIELD(Vec3, position, Vec3(0))
	VCOMPONENT_FIELD(Quat, rotation, Quat(1, 0, 0, 0))
	VCOMPONENT_FIELD(Vec3, scale, Vec3(1))
	VCOMPONENT_END()

	inline Vec3 forward(const Transform &transform) { return transform.rotation * Vec3(0, 0, -1); }
	inline Vec3 right(const Transform &transform) { return transform.rotation * Vec3(1, 0, 0); }
	inline Vec3 up(const Transform &transform) { return transform.rotation * Vec3(0, 1, 0); }
	inline Mat4 view_matrix(const Transform &transform) { return glm::lookAt(transform.position, transform.position + forward(transform), Vec3(0, 1, 0)); }
	inline Mat4 model_matrix(const Transform &transform)
	{
		Mat4 scaling_matrix   = glm::scale(transform.scale);
		Mat4 rotation_matrix  = glm::toMat4(transform.rotation);
		Mat4 translate_matrix = glm::translate(transform.position);
		return translate_matrix * rotation_matrix * scaling_matrix;
	}
} // namespace Vultr