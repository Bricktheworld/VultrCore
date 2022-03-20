#pragma once
#include <filesystem/filesystem.h>
#include <platform/rendering.h>
#include <core/resource_manager/resource_manager.h>

namespace Vultr
{
	struct Mesh
	{
		Resource<Platform::Mesh *> source{};
	};
} // namespace Vultr