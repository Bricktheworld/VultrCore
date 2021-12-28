#include "project.h"
#include <platform/platform.h>

namespace Vultr
{
	Project load_game(const char *location)
	{
		Project project{};
		auto *dl = Platform::dl_open(location);
		ASSERT(dl != nullptr, "Failed to load game");

		auto *use_game_memory = (UseGameMemoryApi)(Platform::dl_load_symbol(dl, USE_GAME_MEMORY_SYMBOL));
		use_game_memory(g_game_memory);

		project.init   = (VultrInitApi)Platform::dl_load_symbol(dl, VULTR_INIT_SYMBOL);
		project.update = (VultrUpdateApi)Platform::dl_load_symbol(dl, VULTR_UPDATE_SYMBOL);
		return project;
	}
} // namespace Vultr
