#pragma once
#include "device.h"
#include "command_pool.h"

namespace Vultr
{
	namespace Platform
	{
		struct UploadContext
		{
			Vulkan::CommandPool cmd_pool{};
			Vulkan::Device *device = nullptr;
		};
	} // namespace Platform

	namespace Vulkan
	{
		Device *get_device(Platform::UploadContext *c);
	}
} // namespace Vultr
