#pragma once
#include "render_context.h"

namespace Vultr
{
	namespace Platform
	{
		struct Texture
		{
			u32 width     = 0;
			u32 height    = 0;
			VkImage image = nullptr;
			VkImageCreateInfo image_info{};
			VmaAllocation allocation = nullptr;
			VmaAllocationInfo alloc_info{};
			TextureFormat format = TextureFormat::RGBA8;
		};

		struct TextureRef
		{
			Texture *ref           = nullptr;
			VkImageView image_view = nullptr;
		};
	} // namespace Platform
	namespace Vulkan
	{
		VkFormat get_vk_texture_format(Vulkan::Device *d, Platform::TextureFormat format);
		VkImageView get_image_view(Device *d, Platform::Texture *texture);
		void destroy_image_view(Device *d, VkImageView view);
	} // namespace Vulkan
} // namespace Vultr