#include "framebuffer.h"
#include "texture.h"
#include "shader.h"

namespace Vultr
{
	namespace Platform
	{
		static VkAttachmentLoadOp get_vk_load_op(LoadOp load_op)
		{
			switch (load_op)
			{
				case LoadOp::DONT_CARE:
					return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				case LoadOp::CLEAR:
					return VK_ATTACHMENT_LOAD_OP_CLEAR;
				case LoadOp::LOAD:
					return VK_ATTACHMENT_LOAD_OP_LOAD;
				default:
					return VK_ATTACHMENT_LOAD_OP_CLEAR;
			}
		}

		static VkAttachmentStoreOp get_vk_store_op(StoreOp store_op)
		{
			switch (store_op)
			{
				case StoreOp::DONT_CARE:
					return VK_ATTACHMENT_STORE_OP_DONT_CARE;
				case StoreOp::STORE:
					return VK_ATTACHMENT_STORE_OP_STORE;
				default:
					return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}
		}

		Framebuffer *init_framebuffer(RenderContext *c, const Vector<AttachmentDescription> &attachments, Option<u32> width, Option<u32> height)
		{
			auto *d    = Vulkan::get_device(c);
			auto *sc   = Vulkan::get_swapchain(c);
			auto *fb   = v_alloc<Framebuffer>();
			fb->width  = width.value_or(sc->extent.width);
			fb->height = height.value_or(sc->extent.height);

			VkAttachmentDescription attachment_descriptions[attachments.size()];
			Vector<VkAttachmentReference> color_attachments{};
			VkAttachmentReference depth_attachment{VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
			Vector<VkImageView> image_views{};

			for (u32 i = 0; i < attachments.size(); i++)
			{
				if (attachments[i].format != TextureFormat::DEPTH)
				{
					color_attachments.push_back({i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
				}
				else
				{
					ASSERT(depth_attachment.attachment == VK_ATTACHMENT_UNUSED, "Multiple depth stencil attachments in use is not allowed!");
					depth_attachment.attachment = i;
				}

				fb->attachments.push_back(init_texture(c, fb->width, fb->height, attachments[i].format, TextureUsage::ATTACHMENT | attachments[i].texture_usage));
				image_views.push_back(fb->attachments[i]->image_view);

				attachment_descriptions[i] = VkAttachmentDescription{
					.format         = fb->attachments[i]->image_info.format,
					.samples        = VK_SAMPLE_COUNT_1_BIT,
					.loadOp         = get_vk_load_op(attachments[i].load_op),
					.storeOp        = get_vk_store_op(attachments[i].store_op),
					.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
					// TODO(Brandon): LOL this is not what I'm supposed to do so fix this later for better performance.
					.finalLayout = VK_IMAGE_LAYOUT_GENERAL,
				};
			}

			// TODO(Brandon): Figure out what the fuck these are supposed to do.
			VkSubpassDependency subpass_dependencies[2] = {{
															   .srcSubpass      = VK_SUBPASS_EXTERNAL,
															   .dstSubpass      = 0,
															   .srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
															   .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
															   .srcAccessMask   = VK_ACCESS_SHADER_READ_BIT,
															   .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
															   .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
														   },
														   {
															   .srcSubpass      = 0,
															   .dstSubpass      = VK_SUBPASS_EXTERNAL,
															   .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
															   .dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
															   .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
															   .dstAccessMask   = VK_ACCESS_SHADER_READ_BIT,
															   .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
														   }}; // namespace Platform

			VkSubpassDescription subpass_description    = {
				   .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
				   .colorAttachmentCount = static_cast<u32>(color_attachments.size()),
				   // TODO(Brandon): Test this.
				   .pColorAttachments       = !color_attachments.empty() ? &color_attachments[0] : nullptr,
				   .pDepthStencilAttachment = &depth_attachment,
            };

			fb->vk_rp_info = {
				.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.attachmentCount = static_cast<u32>(attachments.size()),
				.pAttachments    = attachment_descriptions,
				.subpassCount    = 1,
				.pSubpasses      = &subpass_description,
				.dependencyCount = 2,
				.pDependencies   = subpass_dependencies,
			};

			VK_CHECK(vkCreateRenderPass(d->device, &fb->vk_rp_info, nullptr, &fb->vk_renderpass));

			fb->vk_fb_info = {
				.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass      = fb->vk_renderpass,
				.attachmentCount = static_cast<u32>(image_views.size()),
				.pAttachments    = &image_views[0],
				.width           = fb->width,
				.height          = fb->height,
				.layers          = 1,
			};
			VK_CHECK(vkCreateFramebuffer(d->device, &fb->vk_fb_info, nullptr, &fb->vk_framebuffer));
			return fb;
		}

		void destroy_framebuffer(RenderContext *c, Framebuffer *framebuffer)
		{
			Vulkan::wait_resource_not_in_use(Vulkan::get_swapchain(c), framebuffer);

			Platform::Lock lock(framebuffer->mutex);
			auto *d = Vulkan::get_device(c);

			vkDestroyFramebuffer(d->device, framebuffer->vk_framebuffer, nullptr);
			vkDestroyRenderPass(d->device, framebuffer->vk_renderpass, nullptr);

			for (auto *texture : framebuffer->attachments)
			{
				destroy_texture(c, texture);
			}

			for (auto &[shader, pipelines] : framebuffer->pipelines)
			{
				shader->framebuffers_with_pipelines.remove(framebuffer);
				for (auto &[info, pipeline] : pipelines)
				{
					Vulkan::destroy_pipeline(c, pipeline);
				}
			}
			framebuffer->pipelines.clear();

			v_free(framebuffer);
		}

		Texture *get_attachment_texture(Framebuffer *framebuffer, u32 index)
		{
			ASSERT(index < framebuffer->attachments.size(), "Cannot get texture for attachment %u with only %zu attachments", index, framebuffer->attachments.size());
			return framebuffer->attachments[index];
		}

		u32 get_width(Framebuffer *framebuffer) { return framebuffer->width; }
		u32 get_height(Framebuffer *framebuffer) { return framebuffer->height; }
	} // namespace Platform

	namespace Vulkan
	{
		bool has_depth(Platform::Framebuffer *framebuffer)
		{
			for (auto *texture : framebuffer->attachments)
			{
				if (texture->format == Platform::TextureFormat::DEPTH)
					return true;
			}
			return false;
		}
	} // namespace Vulkan
} // namespace Vultr
