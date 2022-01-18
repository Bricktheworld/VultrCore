#pragma once
#include <types/array.h>
#include <vulkan/vulkan.h>

namespace Vultr
{
	namespace Vulkan
	{
		static constexpr Array<str, 1> VALIDATION_LAYERS = Array({"VK_LAYER_KHRONOS_validation"});
		static constexpr Array<str, 1> DEVICE_EXTENSIONS = Array({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
		static constexpr u32 MAX_FRAMES_IN_FLIGHT        = 2;
	} // namespace Vulkan
} // namespace Vultr