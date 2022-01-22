#pragma once
#include <types/types.h>
#include <types/bitfield.h>

namespace Vultr
{
#define MAX_COMPONENTS 128
	typedef Bitfield<MAX_COMPONENTS> ComponentType;

} // namespace Vultr