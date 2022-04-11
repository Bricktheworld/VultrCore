#pragma once
#include "render_context.h"

namespace Vultr
{
	namespace Platform
	{
		struct Framebuffer
		{
			u32 width  = 0;
			u32 height = 0;
			Vector<Texture *> attachments{};

			VkRenderPass vk_renderpass = nullptr;
			VkRenderPassCreateInfo vk_rp_info{};

			VkFramebuffer vk_framebuffer = nullptr;
			VkFramebufferCreateInfo vk_fb_info{};

			Hashmap<u32, GraphicsPipeline *> pipelines{};
		};
	} // namespace Platform
	namespace Vulkan
	{
		bool has_depth(Platform::Framebuffer *framebuffer);
	}
} // namespace Vultr
