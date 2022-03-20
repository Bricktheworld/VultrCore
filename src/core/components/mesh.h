#pragma once
#include <filesystem/filesystem.h>
#include <platform/rendering.h>
#include <core/resource_allocator/resource_allocator.h>

namespace Vultr
{
	struct Mesh
	{
		Resource<Platform::Mesh *> source{};
	};
} // namespace Vultr