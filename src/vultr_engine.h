#pragma once
#include <platform/rendering.h>

namespace Vultr
{
#ifdef DEBUG
	static constexpr bool IS_DEBUG = true;
#else
	static constexpr bool IS_DEBUG = false;
#endif

	struct Engine
	{
		Platform::RenderContext *context = nullptr;
		Platform::Window *window         = nullptr;
	};

	inline Engine *engine()
	{
		ASSERT(g_game_memory != nullptr && g_game_memory->engine != nullptr, "Game memory not properly initialized!");
		return static_cast<Engine *>(g_game_memory->engine);
	}

	inline void open_windowed(str name, u32 width, u32 height)
	{
		auto *e    = engine();

		e->window  = Platform::open_window(Platform::DisplayMode::WINDOWED, nullptr, name, IS_DEBUG, width, height);
		e->context = Platform::get_render_context(e->window);
	}

	inline void open_fullscreen(str name)
	{
		auto *e    = engine();

		e->window  = Platform::open_window(Platform::DisplayMode::FULLSCREEN, nullptr, name, IS_DEBUG);
		e->context = Platform::get_render_context(e->window);
	}

	inline void open_borderless_windowed(str name)
	{
		auto *e    = engine();

		e->window  = Platform::open_window(Platform::DisplayMode::BORDERLESS_WINDOWED, nullptr, name, IS_DEBUG);
		e->context = Platform::get_render_context(e->window);
	}
} // namespace Vultr
