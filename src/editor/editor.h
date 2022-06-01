#pragma once
#include <core/systems/render_system.h>
#include "project/project.h"
#include "windows/resource_browser.h"
#include "windows/scene_window.h"
#include "windows/component_inspector.h"

namespace ImGui
{
	bool InputText(const Vultr::StringView &label, Vultr::String *string);
}

namespace Vultr
{
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
	};

	struct ErrorMessage
	{
		StringView title{};
		String message{};
	};

	struct ResourceManager
	{
		Platform::UploadContext *upload_context = nullptr;

		Platform::Texture *texture              = nullptr;
		Platform::Texture *cpp_source           = nullptr;
		Platform::Texture *shader               = nullptr;
		Platform::Texture *file                 = nullptr;
		Platform::Texture *folder               = nullptr;
		Platform::Texture *mesh                 = nullptr;
	};

	struct Editor
	{
		ResourceBrowser resource_browser{};
		SceneWindow scene_window{};
		ComponentInspector component_inspector{};

		Option<Entity> selected_entity = None;
		Option<Path> scene_path = None;
		Input::CallbackHandle key_listener{};

		ResourceManager resource_manager{};

		Option<ProgressState> progress_state    = None;
		Option<ErrorMessage> error_message      = None;

		bool started                            = false;
		bool playing                            = false;
		void *game_memory                       = nullptr;

		Platform::Semaphore<1> hot_reload_fence = Platform::Semaphore<1>(1);

		Project project;

		Platform::ImGuiContext *imgui_c = nullptr;
	};

	bool serialize_current_scene(Editor *e);
	bool load_scene(Editor *e, const Path &file);

	ProgressState *begin_progress_bar(Editor *e, const StringView &title);
	void end_progress_bar(Editor *e, ProgressState *progress);

	void display_error(Editor *e, const StringView &title, const String &message);

	void begin_resource_import(Editor *e);

	void scene_window_update(Editor *e, f64 dt);
	void scene_window_draw(Editor *e, RenderSystem::Component *render_system);
	void entity_hierarchy_window_draw(Editor *e);
	void component_inspector_window_draw(Editor *e);
	void resource_window_init(Editor *e);
	void resource_browser_window_draw(Editor *e, Platform::CmdBuffer *cmd);
	void resource_browser_destroy(Editor *e);
	void asset_inpsector_draw(Editor *e);

	void init_windows(Editor *e);
	void update_windows(Editor *e, f64 dt);
	void render_windows(Editor *state, Platform::CmdBuffer *cmd, RenderSystem::Component *render_system, f64 dt);
	void destroy_windows(Editor *e);
} // namespace Vultr