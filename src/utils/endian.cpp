#include "endian.h"

namespace Vultr
{
	namespace Utils
	{
		Endian get_endianness()
		{
			// Credit: https://stackoverflow.com/a/4181991
			int n = 1;
			return (*(char *)&n == 1) ? Endian::LITTLE : Endian::BIG;
		}
	} // namespace Utils
} // namespace Vultr
