#include "texture.h"

namespace Vultr
{
	namespace Vulkan
	{
		VkFormat get_vk_texture_format(Vulkan::Device *d, Platform::TextureFormat format)
		{
			switch (format)
			{
				case Platform::TextureFormat::RGB8:
					return VK_FORMAT_R8G8B8_UNORM;
				case Platform::TextureFormat::RGB16:
					return VK_FORMAT_R16G16B16_UNORM;
				case Platform::TextureFormat::RGBA8:
					return VK_FORMAT_R8G8B8A8_UNORM;
				case Platform::TextureFormat::RGBA16:
					return VK_FORMAT_R16G16B16A16_UNORM;
				case Platform::TextureFormat::SRGB8:
					return VK_FORMAT_R8G8B8_SRGB;
				case Platform::TextureFormat::SRGBA8:
					return VK_FORMAT_R8G8B8A8_SRGB;
				case Platform::TextureFormat::DEPTH:
					return Vulkan::get_supported_depth_format(d);
			}
		}

		VkImageView get_image_view(Device *d, Platform::Texture *texture)
		{
			VkImageViewCreateInfo info{
				.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.format   = texture->image_info.format,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.subresourceRange =
					{
						.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel   = 0,
						.levelCount     = 1,
						.baseArrayLayer = 0,
						.layerCount     = 1,
					},
				.image = texture->image,

			};
			VkImageView view;
			VK_CHECK(vkCreateImageView(d->device, &info, nullptr, &view));
			return view;
		}

		void destroy_image_view(Device *d, VkImageView view) { vkDestroyImageView(d->device, view, nullptr); }
	} // namespace Vulkan
	namespace Platform
	{
		Texture *init_texture(RenderContext *c, u32 width, u32 height, TextureFormat format)
		{
			auto *d         = Vulkan::get_device(c);
			auto *texture   = v_alloc<Texture>();

			texture->width  = width;
			texture->height = height;

			texture->format = format;

			auto extent     = VkExtent3D{
					.width  = width,
					.height = height,
                // TODO(Brandon): Figure out what this value even does.
					.depth = 1,
            };

			VkImageUsageFlags usage_flags = format != TextureFormat::DEPTH ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			texture->image_info           = {
						  .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
						  .imageType   = VK_IMAGE_TYPE_2D,
						  .format      = Vulkan::get_vk_texture_format(d, format),
						  .extent      = extent,
						  .mipLevels   = 1,
						  .arrayLayers = 1,
						  .samples     = VK_SAMPLE_COUNT_1_BIT,
						  .tiling      = VK_IMAGE_TILING_OPTIMAL,
                // NOTE(Brandon): We are just marking all of the flags that MIGHT be used, this should probably be something that we should flag depending on if performance is impacted
						  .usage         = usage_flags | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
						  .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
						  .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
            };

			VmaAllocationCreateInfo alloc_info{
				.usage         = VMA_MEMORY_USAGE_GPU_ONLY,
				.requiredFlags = 0,
			};

			VK_CHECK(vmaCreateImage(d->allocator, &texture->image_info, &alloc_info, &texture->image, &texture->allocation, &texture->alloc_info));

			return texture;
		}

		void destroy_texture(RenderContext *c, Texture *texture)
		{
			ASSERT(texture != nullptr, "Cannot destroy invalid texture!");
			auto *d = Vulkan::get_device(c);
			vmaDestroyImage(d->allocator, texture->image, texture->allocation);
			*texture = {};
			v_free(texture);
		}
	} // namespace Platform
} // namespace Vultr