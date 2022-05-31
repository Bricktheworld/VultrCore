#pragma once
#include <editor/runtime/runtime.h>
#include <editor/project/project.h>
#include <ImGuizmo/ImGuizmo.h>

namespace ImGui
{
	bool InputText(const Vultr::StringView &label, Vultr::String *string);
}

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

	struct ProgressState
	{
		atomic_bool done    = false;
		atomic_u32 total    = 0;
		atomic_u32 progress = 0;

		StringView title{};
		StringView message{};

		ProgressState() = default;
		ProgressState(const ProgressState &other)
		{
			total    = other.total.load();
			progress = other.progress.load();
			title    = other.title;
			message  = other.message;
		}

		ProgressState &operator=(const ProgressState &other)
		{
			total    = other.total.load();
			progress = other.progress.load();
			title    = other.title;
			message  = other.message;
			return *this;
		}
	};

	struct ErrorMessage
	{
		StringView title{};
		String message{};
	};

	struct EditorWindowState
	{
		Vec2 render_window_offset      = Vec2(0);
		Vec2 render_window_size        = Vec2(0);

		Option<Entity> selected_entity = None;
		Camera editor_camera{};
		Transform editor_camera_transform{.position = Vec3(0, 0, 10)};
		bool dockspace_open                   = true;
		ImGuizmo::OPERATION current_operation = ImGuizmo::OPERATION::TRANSLATE;
		ResourceBrowserState resource_browser_state{};

		Buffer component_clipboard{};
		Option<Path> scene_path = None;
		Platform::CallbackHandle key_listener{};

		Platform::Texture *texture              = nullptr;
		Platform::Texture *cpp_source           = nullptr;
		Platform::Texture *shader               = nullptr;
		Platform::Texture *file                 = nullptr;
		Platform::Texture *folder               = nullptr;
		Platform::Texture *mesh                 = nullptr;

		Option<ProgressState> progress_state    = None;
		Option<ErrorMessage> error_message      = None;

		bool started                            = false;
		bool playing                            = false;
		void *game_memory                       = nullptr;

		Platform::Semaphore<1> hot_reload_fence = Platform::Semaphore<1>(1);
	};

	bool serialize_current_scene(EditorWindowState *state);
	bool load_scene(EditorWindowState *state, const Path &file);

	ProgressState *begin_progress_bar(EditorWindowState *state, const StringView &title);
	void end_progress_bar(EditorWindowState *state, ProgressState *progress);

	void display_error(EditorWindowState *state, const StringView &title, const String &message);

	void begin_resource_import(Project *project, EditorWindowState *state);

	void scene_window_update(EditorWindowState *state, f64 dt);
	void scene_window_draw(RenderSystem::Component *render_system, Project *project, EditorWindowState *state, EditorRuntime *runtime);
	void entity_hierarchy_window_draw(Project *project, EditorWindowState *state);
	void component_inspector_window_draw(Project *project, EditorWindowState *state);
	void resource_window_init(Project *project, EditorWindowState *state);
	void resource_browser_window_draw(Project *project, EditorWindowState *state);
	void asset_inpsector_draw(Project *project, EditorWindowState *state);
	void init_windows(EditorRuntime *runtime, Project *project, EditorWindowState *state);
	void update_windows(EditorWindowState *state, f64 dt);
	void render_windows(Platform::CmdBuffer *cmd, RenderSystem::Component *render_system, Project *project, EditorWindowState *state, EditorRuntime *runtime, f64 dt);
	void destroy_windows(EditorWindowState *state);
} // namespace Vultr