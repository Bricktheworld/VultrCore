#pragma once
#include "vultr_memory.h"
#include "vultr_ecs.h"
#include "vultr_engine.h"

namespace Vultr
{
	static constexpr str VULTR_GAMEPLAY_LIBRARY_LOCATION = "build/libGameplay.so";
	void init();
	void open_window(Platform::DisplayMode display_mode, str name);
	void destroy();
} // namespace Vultr
