#pragma once
#include <vultr.h>
#include <platform/platform.h>
#include <filesystem/path.h>

namespace Vultr
{
	struct Project
	{
		Platform::DLL dll{};
		VultrInitApi init{};
		VultrUpdateApi update{};
	};

	ErrorOr<Project> load_game(const Path &location);
} // namespace Vultr