#pragma once
#include <vultr.h>

namespace Vultr
{
	struct Project
	{
		VultrInitApi init;
		VultrUpdateApi update;
	};
	Project load_game(const char *location);
} // namespace Vultr
