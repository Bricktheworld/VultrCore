#pragma once
#include <vultr.h>
#include <ImGuizmo/ImGuizmo.h>

namespace Vultr
{
	struct SceneWindow
	{
		Vec2 window_offset             = Vec2(0);
		Vec2 window_size               = Vec2(0);

		Camera editor_camera{};
		Transform editor_camera_transform{.position = Vec3(0, 0, 10)};
		ImGuizmo::OPERATION current_operation = ImGuizmo::OPERATION::TRANSLATE;


	};
} // namespace Vultr