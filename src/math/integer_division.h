#pragma once
#include <types/types.h>

namespace Vultr
{
	inline u8 int_ceil_divide(u8 x, u8 y) { return x / y + (x % y != 0); }
	inline u16 int_ceil_divide(u16 x, u16 y) { return x / y + (x % y != 0); }
	inline u32 int_ceil_divide(u32 x, u32 y) { return x / y + (x % y != 0); }
	inline u64 int_ceil_divide(u64 x, u64 y) { return x / y + (x % y != 0); }
} // namespace Vultr