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
				case Platform::TextureFormat::RGB8_CUBEMAP:
					return VK_FORMAT_R8G8B8_UNORM;
				case Platform::TextureFormat::RGB16:
				case Platform::TextureFormat::RGB16_CUBEMAP:
					return VK_FORMAT_R16G16B16_UNORM;
				case Platform::TextureFormat::RGBA8:
				case Platform::TextureFormat::RGBA8_CUBEMAP:
					return VK_FORMAT_R8G8B8A8_UNORM;
				case Platform::TextureFormat::RGBA16:
				case Platform::TextureFormat::RGBA16_CUBEMAP:
					return VK_FORMAT_R16G16B16A16_UNORM;
				case Platform::TextureFormat::SRGB8:
				case Platform::TextureFormat::SRGB8_CUBEMAP:
					return VK_FORMAT_R8G8B8_SRGB;
				case Platform::TextureFormat::SRGBA8:
				case Platform::TextureFormat::SRGBA8_CUBEMAP:
					return VK_FORMAT_R8G8B8A8_SRGB;
				case Platform::TextureFormat::DEPTH:
					return Vulkan::get_supported_depth_format(d);
				default:
					return VK_FORMAT_R8G8B8_UNORM;
			}
		}
	} // namespace Vulkan
	namespace Platform
	{
		static bool is_cubemap(TextureFormat format)
		{
			switch (format)
			{
				case Platform::TextureFormat::RGB8_CUBEMAP:
				case Platform::TextureFormat::RGB16_CUBEMAP:
				case Platform::TextureFormat::RGBA8_CUBEMAP:
				case Platform::TextureFormat::RGBA16_CUBEMAP:
				case Platform::TextureFormat::SRGB8_CUBEMAP:
				case Platform::TextureFormat::SRGBA8_CUBEMAP:
					return true;
				default:
					return false;
			}
		}
		static VkImageCreateFlags vk_get_cubemap_flag(Platform::TextureFormat format) { return is_cubemap(format) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0; }
		static Texture *init_texture(Vulkan::Device *d, u32 width, u32 height, TextureFormat format, u16 texture_usage)
		{
			auto *texture          = v_alloc<Texture>();

			texture->width         = width;
			texture->height        = height;

			texture->format        = format;
			texture->texture_usage = texture_usage;

			auto extent            = VkExtent3D{
						   .width  = width,
						   .height = height,
                // TODO(Brandon): Figure out what this value even does.
						   .depth = 1,
            };

			VkImageUsageFlags usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT;
			if (texture_usage & TextureUsage::ATTACHMENT)
			{
				if (format == TextureFormat::DEPTH)
				{
					usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				}
				else
				{
					usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				}
			}
			if (texture_usage & TextureUsage::TEXTURE)
			{
				usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}
			if (texture_usage & TextureUsage::STORAGE)
			{
				usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}
			bool is_skybox      = is_cubemap(format);

			texture->image_info = {
				.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType     = VK_IMAGE_TYPE_2D,
				.format        = Vulkan::get_vk_texture_format(d, format),
				.extent        = extent,
				.mipLevels     = 1,
				.arrayLayers   = is_skybox ? 6U : 1U,
				.samples       = VK_SAMPLE_COUNT_1_BIT,
				.tiling        = VK_IMAGE_TILING_OPTIMAL,
				.usage         = usage_flags,
				.sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.flags         = vk_get_cubemap_flag(format),
			};

			VmaAllocationCreateInfo alloc_info{
				.usage         = VMA_MEMORY_USAGE_GPU_ONLY,
				.requiredFlags = 0,
			};

			VK_CHECK(vmaCreateImage(d->allocator, &texture->image_info, &alloc_info, &texture->image, &texture->allocation, &texture->alloc_info));

			texture->sampler_info = {
				.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
				.magFilter     = VK_FILTER_LINEAR,
				.minFilter     = VK_FILTER_LINEAR,
				.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR,
				.addressModeU  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				.addressModeV  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				.addressModeW  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				.mipLodBias    = 0.0f,
				.maxAnisotropy = 1.0f,
				.minLod        = 0.0f,
				.maxLod        = 0.0f,
				.borderColor   = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			};

			VK_CHECK(vkCreateSampler(d->device, &texture->sampler_info, nullptr, &texture->sampler));

			texture->image_view_info = {
				.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image    = texture->image,
				.viewType = is_skybox ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
				.format   = texture->image_info.format,
				.subresourceRange =
					{
						.aspectMask     = texture->format != Platform::TextureFormat::DEPTH ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT,
						.baseMipLevel   = 0,
						.levelCount     = 1,
						.baseArrayLayer = 0,
						.layerCount     = is_skybox ? 6U : 1U,
					},
			};
			VK_CHECK(vkCreateImageView(d->device, &texture->image_view_info, nullptr, &texture->image_view));

			texture->layout            = VK_IMAGE_LAYOUT_GENERAL;
			texture->cached_texture_id = nullptr;

			return texture;
		}

		Texture *init_texture(RenderContext *c, u32 width, u32 height, TextureFormat format, u16 texture_usage) { return init_texture(Vulkan::get_device(c), width, height, format, texture_usage); }
		Texture *init_texture(UploadContext *c, u32 width, u32 height, TextureFormat format, u16 texture_usage) { return init_texture(Vulkan::get_device(c), width, height, format, texture_usage); }

		Texture *init_white_texture(UploadContext *c)
		{
			auto *texture = init_texture(c, 1, 1, TextureFormat::RGBA8, TextureUsage::TEXTURE);
			byte data[4]  = {255, 255, 255, 255};
			fill_texture(c, texture, data, get_pixel_size(texture->format));
			return texture;
		}

		Texture *init_black_texture(UploadContext *c)
		{
			auto *texture = init_texture(c, 1, 1, TextureFormat::RGBA8, TextureUsage::TEXTURE);
			byte data[4]  = {0};
			fill_texture(c, texture, data, get_pixel_size(texture->format));
			return texture;
		}

		Texture *init_normal_texture(UploadContext *c)
		{
			auto *texture = init_texture(c, 1, 1, TextureFormat::RGBA8, TextureUsage::TEXTURE);
			byte data[4]  = {128, 128, 255, 0};
			fill_texture(c, texture, data, get_pixel_size(texture->format));
			return texture;
		}

		void fill_texture(UploadContext *c, Texture *texture, byte *data, u32 size)
		{
			auto *d             = Vulkan::get_device(c);

			auto staging_buffer = alloc_buffer(d, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

			fill_buffer(d, &staging_buffer, data, size);

			bool is_skybox = is_cubemap(texture->format);
			VkImageSubresourceRange range{
				.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel   = 0,
				.levelCount     = 1,
				.baseArrayLayer = 0,
				.layerCount     = is_skybox ? 6U : 1U,
			};

			VkImageMemoryBarrier barrier_to_transfer = {
				.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask    = 0,
				.dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
				.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.image            = texture->image,
				.subresourceRange = range,
			};

			auto cmd = begin_cmd_buffer(d, &c->cmd_pool);
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_to_transfer);

			VkBufferImageCopy copy = {
				.bufferOffset      = 0,
				.bufferRowLength   = 0,
				.bufferImageHeight = 0,
				.imageSubresource =
					{
						.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
						.mipLevel       = 0,
						.baseArrayLayer = 0,
						.layerCount     = is_skybox ? 6U : 1U,
					},
				.imageExtent = texture->image_info.extent,
			};

			vkCmdCopyBufferToImage(cmd, staging_buffer.buffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

			VkImageMemoryBarrier barrier_to_readable{
				.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
				.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT,
				.oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.newLayout        = texture->layout,
				.image            = texture->image,
				.subresourceRange = range,
			};

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_to_readable);

			end_cmd_buffer(cmd, &c->cmd_pool);

			VkSubmitInfo submit_info{
				.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.commandBufferCount = 1,
				.pCommandBuffers    = &cmd,
			};

			auto fence = c->cmd_pool.fence;
			graphics_queue_submit(d, 1, &submit_info, fence);

			VK_CHECK(vkWaitForFences(d->device, 1, &fence, VK_TRUE, U64Max));
			recycle_cmd_pool(d, &c->cmd_pool);

			Vulkan::unsafe_free_buffer(d, &staging_buffer);
		}

		u32 get_width(Texture *texture) { return texture->width; }
		u32 get_height(Texture *texture) { return texture->height; }

		static void destroy_texture(Vulkan::SwapChain *sc, Texture *texture)
		{
			ASSERT(texture != nullptr, "Cannot destroy invalid texture!");
			Vulkan::wait_resource_not_in_use(sc, texture);

			auto *d = Vulkan::get_device(sc);

			vkDestroyImageView(d->device, texture->image_view, nullptr);
			vkDestroySampler(d->device, texture->sampler, nullptr);
			vmaDestroyImage(d->allocator, texture->image, texture->allocation);
			texture->image_view = nullptr;
			texture->sampler    = nullptr;
			texture->image      = nullptr;

			if (texture->cached_texture_id != nullptr)
				imgui_free_texture_id(texture);
			v_free(texture);
		}

		void destroy_texture(RenderContext *c, Texture *texture) { destroy_texture(Vulkan::get_swapchain(c), texture); }
	} // namespace Platform
} // namespace Vultr
