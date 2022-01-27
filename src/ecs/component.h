#pragma once
#include <types/types.h>
#include <types/bitfield.h>

namespace Vultr
{
	static constexpr size_t MAX_COMPONENTS = 128;
	typedef Bitfield<MAX_COMPONENTS> Signature;

} // namespace Vultr