#pragma once
#include <types/types.h>
#include "gpu_buffer.h"

namespace Vultr
{
	namespace Platform
	{
		struct IndexBuffer
		{
			Vulkan::GpuBuffer index_buffer;
		};
	} // namespace Platform
} // namespace Vultr
