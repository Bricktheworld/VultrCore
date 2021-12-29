#include "project.h"
#include <platform/platform.h>

namespace Vultr
{
	ErrorOr<Project> load_game(const char *location)
	{
		Project project{};
		UNWRAP(project.dll, Platform::dl_open(location));

		UNWRAP(auto use_game_memory, Platform::dl_load_symbol<UseGameMemoryApi>(&project.dll, USE_GAME_MEMORY_SYMBOL));
		use_game_memory(g_game_memory);

		UNWRAP(project.init, Platform::dl_load_symbol<VultrInitApi>(&project.dll, VULTR_INIT_SYMBOL));
		UNWRAP(project.update, Platform::dl_load_symbol<VultrUpdateApi>(&project.dll, VULTR_UPDATE_SYMBOL));

		return project;
	}
} // namespace Vultr
