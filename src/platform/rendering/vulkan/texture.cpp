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
				default:
					return VK_FORMAT_R8G8B8_UNORM;
			}
		}
	} // namespace Vulkan
	namespace Platform
	{
		static Texture *init_texture(Vulkan::Device *d, u32 width, u32 height, TextureFormat format)
		{
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
						  .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
						  .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
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
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format   = texture->image_info.format,
				.subresourceRange =
					{
						.aspectMask     = texture->format != Platform::TextureFormat::DEPTH ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT,
						.baseMipLevel   = 0,
						.levelCount     = 1,
						.baseArrayLayer = 0,
						.layerCount     = 1,
					},
			};

			texture->layout = VK_IMAGE_LAYOUT_GENERAL;
			VK_CHECK(vkCreateImageView(d->device, &texture->image_view_info, nullptr, &texture->image_view));

			texture->cached_texture_id = nullptr;

			return texture;
		}

		Texture *init_texture(RenderContext *c, u32 width, u32 height, TextureFormat format) { return init_texture(Vulkan::get_device(c), width, height, format); }
		Texture *init_texture(UploadContext *c, u32 width, u32 height, TextureFormat format) { return init_texture(Vulkan::get_device(c), width, height, format); }

		Texture *init_white_texture(UploadContext *c)
		{
			auto *texture = init_texture(c, 1, 1, TextureFormat::RGBA8);
			byte data[4]  = {255, 255, 255, 255};
			fill_texture(c, texture, data);
			return texture;
		}

		Texture *init_black_texture(UploadContext *c)
		{
			auto *texture = init_texture(c, 1, 1, TextureFormat::RGBA8);
			byte data[4]  = {0};
			fill_texture(c, texture, data);
			return texture;
		}

		Texture *init_normal_texture(UploadContext *c)
		{
			auto *texture = init_texture(c, 1, 1, TextureFormat::RGBA8);
			byte data[4]  = {128, 128, 255, 0};
			fill_texture(c, texture, data);
			return texture;
		}

		void fill_texture(UploadContext *c, Texture *texture, byte *data)
		{
			auto *d = Vulkan::get_device(c);
			size_t size;
			switch (texture->format)
			{
				case TextureFormat::RGB8:
					size = 3 * 2;
					break;
				case TextureFormat::RGB16:
					size = 3;
					break;
				case TextureFormat::RGBA8:
					size = 4;
					break;
				case TextureFormat::RGBA16:
					size = 4 * 2;
					break;
				case TextureFormat::SRGB8:
					size = 3;
					break;
				case TextureFormat::SRGBA8:
					size = 4;
					break;
				case TextureFormat::DEPTH:
					size = 1;
					break;
			}

			auto staging_buffer = alloc_buffer(d, texture->width * texture->height * size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

			fill_buffer(d, &staging_buffer, data, texture->width * texture->height * size);

			VkImageSubresourceRange range{
				.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel   = 0,
				.levelCount     = 1,
				.baseArrayLayer = 0,
				.layerCount     = 1,
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
						.layerCount     = 1,
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

			VK_CHECK(vkResetFences(d->device, 1, &c->cmd_pool.fence));
			graphics_queue_submit(d, 1, &submit_info, c->cmd_pool.fence);

			VK_CHECK(vkWaitForFences(d->device, 1, &c->cmd_pool.fence, VK_TRUE, UINT64_MAX));

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
			*texture = {};
			v_free(texture);
		}

		void destroy_texture(RenderContext *c, Texture *texture) { destroy_texture(Vulkan::get_swapchain(c), texture); }
	} // namespace Platform
} // namespace Vultr