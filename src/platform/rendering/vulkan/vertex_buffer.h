#pragma once
#include <types/vector.h>
#include <utils/transfer.h>
#include "gpu_buffer.h"
#include "render_context.h"

namespace Vultr
{
	namespace Platform
	{
		struct VertexBuffer
		{
			Vulkan::GpuBuffer vertex_buffer{};
		};

	} // namespace Platform
} // namespace Vultr
