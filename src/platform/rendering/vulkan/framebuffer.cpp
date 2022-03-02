#include "framebuffer.h"
#include "texture.h"

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

				fb->attachments.push_back(init_texture(c, fb->width, fb->height, attachments[i].format));
				fb->vk_image_views.push_back(Vulkan::get_image_view(d, fb->attachments[i]));

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
															   .srcSubpass    = VK_SUBPASS_EXTERNAL,
															   .dstSubpass    = 0,
															   .srcStageMask  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
															   .dstStageMask  = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
															   .srcAccessMask = 0,
															   .dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
																				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
															   .dependencyFlags = 0,
														   },
														   {
															   .srcSubpass    = 0,
															   .dstSubpass    = VK_SUBPASS_EXTERNAL,
															   .srcStageMask  = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
															   .dstStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
															   .srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
																				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
															   .dstAccessMask   = 0,
															   .dependencyFlags = 0,
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
				.attachmentCount = static_cast<u32>(fb->vk_image_views.size()),
				.pAttachments    = &fb->vk_image_views[0],
				.width           = fb->width,
				.height          = fb->height,
				.layers          = 1,
			};
			VK_CHECK(vkCreateFramebuffer(d->device, &fb->vk_fb_info, nullptr, &fb->vk_framebuffer));
			return fb;
		}

		void destroy_framebuffer(RenderContext *c, Framebuffer *framebuffer)
		{
			auto *d = Vulkan::get_device(c);

			vkDestroyFramebuffer(d->device, framebuffer->vk_framebuffer, nullptr);
			vkDestroyRenderPass(d->device, framebuffer->vk_renderpass, nullptr);

			for (auto image_view : framebuffer->vk_image_views)
			{
				Vulkan::destroy_image_view(d, image_view);
			}

			for (auto *texture : framebuffer->attachments)
			{
				destroy_texture(c, texture);
			}

			v_free(framebuffer);
		}
	} // namespace Platform
} // namespace Vultr