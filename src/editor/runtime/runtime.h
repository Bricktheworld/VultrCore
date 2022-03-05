#pragma once
#include <core/systems/render_system.h>
#include <core/systems/resource_system.h>

namespace Vultr
{
	struct EditorRuntime
	{
		Platform::ImGuiContext *imgui_c            = nullptr;
		RenderSystem::Component *render_system     = nullptr;
		ResourceSystem::Component *resource_system = nullptr;
	};
} // namespace Vultr