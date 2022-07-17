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
			u16 texture_usage    = TextureUsage::TEXTURE;
			VkImageLayout layout{};

			ImTextureID cached_texture_id = nullptr;

			bool operator==(const Texture &other) const
			{
				return width == other.width && height == other.height && image == other.image && sampler == other.sampler && image_view == other.image_view && allocation == other.allocation &&
					   format == other.format && texture_usage == other.texture_usage;
			}
		};

	} // namespace Platform
	namespace Vulkan
	{
		VkFormat get_vk_texture_format(Vulkan::Device *d, Platform::TextureFormat format);
	} // namespace Vulkan

	template <>
	struct Traits<Platform::Texture> : GenericTraits<Platform::Texture>
	{
		static u64 hash(const Platform::Texture &texture)
		{
			byte data[sizeof(texture.image) + sizeof(texture.sampler) + sizeof(texture.image_view)]{};

			memcpy(data, &texture.image, sizeof(texture.image));
			memcpy(data + sizeof(texture.image), &texture.sampler, sizeof(texture.sampler));
			memcpy(data + sizeof(texture.image) + sizeof(texture.sampler), &texture.image_view, sizeof(texture.image_view));

			return string_hash((const char *)(data), sizeof(data));
		}
	};
} // namespace Vultr
