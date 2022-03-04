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
			VkSampler sampler = nullptr;
			VkSamplerCreateInfo sampler_info{};
			VkImageView image_view{};
			VkImageViewCreateInfo image_view_info{};
			VmaAllocation allocation = nullptr;
			VmaAllocationInfo alloc_info{};
			TextureFormat format = TextureFormat::RGBA8;
			VkImageLayout layout{};

			ImTextureID cached_texture_id = nullptr;

			bool operator==(const Texture &other) const
			{
				return width == other.width && height == other.height && image == other.image && sampler == other.sampler && image_view == other.image_view && allocation == other.allocation &&
					   format == other.format;
			}
		};
	} // namespace Platform
	namespace Vulkan
	{
		VkFormat get_vk_texture_format(Vulkan::Device *d, Platform::TextureFormat format);
	} // namespace Vulkan
} // namespace Vultr