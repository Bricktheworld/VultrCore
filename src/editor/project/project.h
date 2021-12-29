#pragma once
#include <vultr.h>
#include <platform/platform.h>

namespace Vultr
{
	struct Project
	{
		Platform::DLL dll;
		VultrInitApi init;
		VultrUpdateApi update;
	};
	//	ErrorOr<Project> load_game(const char *location);
} // namespace Vultr
