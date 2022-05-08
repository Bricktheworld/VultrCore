#pragma once

#include "editor/runtime/runtime.h"
#include <ImGuizmo/ImGuizmo.h>
namespace Vultr
{
	struct ResourceBrowserState
	{
		Vector<Path> files{};
		Vector<Path> dirs{};
		Path current_dir{};
		Option<u32> selected_index = None;
		atomic_bool need_refresh   = true;
	};

	struct ResourceImportState
	{
		atomic_bool importing      = false;
		atomic_u32 total           = 0;
		atomic_u32 progress        = 0;
		Option<Error> import_error = None;
	};

	struct EditorWindowState
	{
		ResourceImportState resource_import_state;
		Option<Entity> selected_entity = None;
		Camera editor_camera{};
		Transform editor_camera_transform{.position = Vec3(0, 0, 10)};
		bool dockspace_open                   = true;
		ImGuizmo::OPERATION current_operation = ImGuizmo::OPERATION::TRANSLATE;
		ResourceBrowserState resource_browser_state{};

		Buffer component_clipboard{};
		Option<Path> scene_path = None;
		Platform::CallbackHandle key_listener{};

		Platform::Texture *texture    = nullptr;
		Platform::Texture *cpp_source = nullptr;
		Platform::Texture *shader     = nullptr;
		Platform::Texture *file       = nullptr;
		Platform::Texture *folder     = nullptr;
		Platform::Texture *mesh       = nullptr;
	};

	void scene_window_update(EditorWindowState *state, f64 dt);
	void scene_window_draw(EditorWindowState *state, EditorRuntime *runtime);
	void entity_hierarchy_window_draw(Project *project, EditorWindowState *state);
	void component_inspector_window_draw(Project *project, EditorWindowState *state);
	void resource_browser_window_draw(Project *project, EditorWindowState *state);
	void init_windows(EditorRuntime *runtime, Project *project, EditorWindowState *state);
	void update_windows(EditorWindowState *state, f64 dt);
	void render_windows(Platform::CmdBuffer *cmd, Project *project, EditorWindowState *state, EditorRuntime *runtime, f64 dt);
	void destroy_windows(EditorWindowState *state);
	void begin_resource_import(Project *project, EditorWindowState *state);
} // namespace Vultr