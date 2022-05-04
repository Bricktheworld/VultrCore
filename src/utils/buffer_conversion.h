#pragma once
#include <types/buffer.h>
#include "endian.h"

namespace Vultr
{
	namespace Utils
	{
		inline u64 u64_from_buf(const Buffer &buffer, Endian src_endianess) { ASSERT(buffer.size() == 8, "Buffer is not exactly 8 bytes so it cannot be converted to u64"); }
	} // namespace Utils
} // namespace Vultr