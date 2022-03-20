#pragma once
#include <core/systems/render_system.h>

namespace Vultr
{
	struct EditorRuntime
	{
		Platform::ImGuiContext *imgui_c         = nullptr;
		Platform::UploadContext *upload_context = nullptr;
		RenderSystem::Component *render_system  = nullptr;
	};
} // namespace Vultr