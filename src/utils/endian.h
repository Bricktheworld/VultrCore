#pragma once
#include <types/types.h>

namespace Vultr
{
	namespace Utils
	{
		enum struct Endian : u8
		{
			LITTLE = 0x0,
			BIG    = 0x1,
		};

		Endian get_endianness();
	} // namespace Utils
} // namespace Vultr