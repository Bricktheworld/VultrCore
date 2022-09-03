#pragma once
#include <filesystem/filesystem.h>
#include <platform/rendering.h>
#include <core/resource_allocator/resource_allocator.h>

namespace Vultr
{
	VCOMPONENT_BEGIN(Mesh)
	VCOMPONENT_FIELD(Resource<Platform::Mesh *>, source, {})
	VCOMPONENT_END()
} // namespace Vultr