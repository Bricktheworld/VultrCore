#pragma once
#include <vultr.h>
#include <platform/platform.h>
#include <filesystem/filesystem.h>

namespace Vultr
{
	struct Project
	{
		Platform::DLL dll{};
		Path resource_dir{};
		Path build_dir{};
		VultrInitApi init{};
		VultrUpdateApi update{};
		VultrDestroyApi destroy{};
	};

	enum struct ResourceType
	{
		TEXTURE,
		SHADER,
		MATERIAL,
		MESH,
		CPP_SRC,
		OTHER,
	};

	ResourceType get_resource_type(const Path &path);
	ErrorOr<Project> load_game(const Path &build_dir, const Path &resource_dir);
	ErrorOr<void> import_resource_dir(Project *project);
} // namespace Vultr