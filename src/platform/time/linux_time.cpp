#pragma once
#include <types/types.h>

namespace Vultr
{
	namespace Platform
	{
		u64 current_ms()
		{
			struct timespec spec;
			ASSERT(clock_gettime(CLOCK_REALTIME, &spec) == 0, "Failed to get current time!");

			return spec.tv_sec * 1e3 + spec.tv_nsec * 1e-6;
		}
	} // namespace Platform
} // namespace Vultr
