#pragma once

#include "editor/runtime/runtime.h"
#include <ImGuizmo/ImGuizmo.h>
namespace Vultr
{
	struct EditorWindowState
	{
		Option<Entity> selected_entity = None;
		Camera editor_camera{};
		Transform editor_camera_transform{.position = Vec3(0, 0, 10)};
		bool dockspace_open = true;
		ImGuizmo::OPERATION current_operation = ImGuizmo::OPERATION::TRANSLATE;
	};

	void scene_window_update(EditorWindowState *state, f64 dt);
	void scene_window_draw(EditorWindowState *state);
	void entity_hierarchy_window_draw(EditorWindowState *state);
	void component_inspector_window_draw(EditorWindowState *state);
	void update_windows(EditorWindowState *state, f64 dt);
	void render_windows(Platform::CmdBuffer *cmd, EditorWindowState *state, EditorRuntime *runtime, f64 dt);
} // namespace Vultr