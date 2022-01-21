#pragma once
#include <types/types.h>
#include <types/bitfield.h>

namespace Vultr
{
#define MAX_COMPONENTS 128
	typedef Bitfield<128> ComponentType;

} // namespace Vultr