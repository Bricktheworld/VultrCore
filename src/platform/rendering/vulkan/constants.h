#pragma once
#include <types/types.h>
#include <types/array.h>
#include <vulkan/vulkan.h>

namespace Vultr
{
	namespace Vulkan
	{
		static constexpr Array<str, 1> VALIDATION_LAYERS = Array({"VK_LAYER_KHRONOS_validation"});
		static constexpr Array<str, 1> DEVICE_EXTENSIONS = Array({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
		static constexpr u32 MAX_FRAMES_IN_FLIGHT        = 2;

		inline str vk_err_string(VkResult errorCode)
		{
			switch (errorCode)
			{
#define STR(r)                                                                                                                                                                                                        \
	case VK_##r:                                                                                                                                                                                                      \
		return #r

				STR(NOT_READY);
				STR(TIMEOUT);
				STR(EVENT_SET);
				STR(EVENT_RESET);
				STR(INCOMPLETE);
				STR(ERROR_OUT_OF_HOST_MEMORY);
				STR(ERROR_OUT_OF_DEVICE_MEMORY);
				STR(ERROR_INITIALIZATION_FAILED);
				STR(ERROR_DEVICE_LOST);
				STR(ERROR_MEMORY_MAP_FAILED);
				STR(ERROR_LAYER_NOT_PRESENT);
				STR(ERROR_EXTENSION_NOT_PRESENT);
				STR(ERROR_FEATURE_NOT_PRESENT);
				STR(ERROR_INCOMPATIBLE_DRIVER);
				STR(ERROR_TOO_MANY_OBJECTS);
				STR(ERROR_FORMAT_NOT_SUPPORTED);
				STR(ERROR_SURFACE_LOST_KHR);
				STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
				STR(SUBOPTIMAL_KHR);
				STR(ERROR_OUT_OF_DATE_KHR);
				STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
				STR(ERROR_VALIDATION_FAILED_EXT);
				STR(ERROR_INVALID_SHADER_NV);

#undef STR
				default:
					return "UNKNOWN_ERROR";
			}
		}
#define VK_CHECK(statement)                                                                                                                                                                                           \
	{                                                                                                                                                                                                                 \
		auto result = (statement);                                                                                                                                                                                    \
		if (result != VK_SUCCESS)                                                                                                                                                                                     \
			THROW("Vulkan check went wrong %s", Vulkan::vk_err_string(result));                                                                                                                                       \
	}
	} // namespace Vulkan
} // namespace Vultr