#include <filesystem/filestream.h>
#include "render_context.h"
#include "gpu_buffer.h"
#include "framebuffer.h"
#include "texture.h"

namespace Vultr
{
	namespace Vulkan
	{
		// TODO(Brandon): Don't do this here.
		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_cb(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
													   const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
		{
			if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			{
				fprintf(stderr, "(Vulkan): %s\n", callback_data->pMessage);
			}
			else
			{
				fprintf(stdout, "(Vulkan): %s\n", callback_data->pMessage);
			}

			return VK_FALSE;
		}

	} // namespace Vulkan
	namespace Platform
	{
		RenderContext *init_render_context(const Platform::Window *window, bool debug)
		{
			auto *c = static_cast<RenderContext *>(persist_alloc(sizeof(RenderContext)));
			PRODUCTION_ASSERT(c != nullptr, "Failed to allocate render context!");
			new (c) RenderContext();

			auto device = Vulkan::init_device(window, debug, Vulkan::debug_cb);
			Vulkan::init_swapchain(&device, window, &c->swap_chain);

			auto *upload_context = init_upload_context(c);
			c->white_texture     = Platform::init_white_texture(upload_context);
			c->black_texture     = Platform::init_black_texture(upload_context);
			c->normal_texture    = Platform::init_normal_texture(upload_context);
			destroy_upload_context(upload_context);
			return c;
		}

		ErrorOr<CmdBuffer *> begin_cmd_buffer(const Platform::Window *window)
		{
			auto *c  = get_render_context(window);
			auto res = Vulkan::acquire_swapchain(&c->swap_chain);
			if (res.has_value())
			{
				auto [image_index, frame, framebuffer] = res.value();
				auto *cmd_pool                         = &frame->cmd_pool;

				Vulkan::recycle_cmd_pool(Vulkan::get_device(c), cmd_pool);
				Vulkan::get_swapchain(c)->cmd_buffer_resource_semaphore.acquire();
				auto cmd             = Vulkan::begin_cmd_buffer(Vulkan::get_device(c), cmd_pool);

				auto *buf            = v_alloc<CmdBuffer>();
				buf->render_context  = c;
				buf->cmd_buffer      = cmd;
				buf->frame           = frame;
				buf->image_index     = image_index;
				buf->out_framebuffer = framebuffer;

				return buf;
			}
			else
			{
				Vulkan::recreate_swapchain(Vulkan::get_swapchain(c), window);
				return Error("Need to recreate swap chain.");
			}
		}

		static void begin_vk_framebuffer(VkCommandBuffer cmd, VkRenderPass render_pass, VkFramebuffer framebuffer, u32 width, u32 height, const Vector<VkClearValue> &clear_colors)
		{
			VkViewport viewport{
				.x        = 0,
				.y        = static_cast<f32>(height),
				.width    = static_cast<f32>(width),
				.height   = -static_cast<f32>(height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f,
			};

			vkCmdSetViewport(cmd, 0, 1, &viewport);
			VkExtent2D extent = {width, height};

			VkRect2D scissor{
				.offset = {.x = 0, .y = 0},
				.extent = extent,
			};

			vkCmdSetScissor(cmd, 0, 1, &scissor);

			VkRenderPassBeginInfo render_pass_info{
				.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.renderPass  = render_pass,
				.framebuffer = framebuffer,
				.renderArea =
					{
						.offset = {0, 0},
						.extent = extent,
					},
				.clearValueCount = static_cast<u32>(clear_colors.size()),
				.pClearValues    = &clear_colors[0],
			};

			vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		}

		void begin_window_framebuffer(CmdBuffer *cmd, Vec4 clear_color)
		{
			auto *sc               = Vulkan::get_swapchain(cmd->render_context);
			const auto clear_value = VkClearValue{.color = {.float32 = {clear_color.x, clear_color.y, clear_color.z, clear_color.w}}};
			begin_vk_framebuffer(cmd->cmd_buffer, sc->render_pass, cmd->out_framebuffer, sc->extent.width, sc->extent.height, Vector({clear_value}));
		}

		void begin_framebuffer(CmdBuffer *cmd, Framebuffer *framebuffer, Vec4 clear_color)
		{
			Vector<VkClearValue> clear_colors{};
			for (auto *image : framebuffer->attachments)
			{
				if (image->format == TextureFormat::DEPTH)
				{
					clear_colors.push_back(VkClearValue{.depthStencil = {1.0f, 0}});
				}
				else
				{
					clear_colors.push_back(VkClearValue{.color = {.float32 = {clear_color.x, clear_color.y, clear_color.z, clear_color.w}}});
				}
			}

			begin_vk_framebuffer(cmd->cmd_buffer, framebuffer->vk_renderpass, framebuffer->vk_framebuffer, framebuffer->width, framebuffer->height, clear_colors);
			Vulkan::depend_resource(cmd, framebuffer);
		}
		void end_framebuffer(CmdBuffer *cmd) { vkCmdEndRenderPass(cmd->cmd_buffer); }

		void end_cmd_buffer(CmdBuffer *cmd)
		{
			auto *c = cmd->render_context;
			Vulkan::end_cmd_buffer(cmd->cmd_buffer, &cmd->frame->cmd_pool);
			Vulkan::submit_swapchain(Vulkan::get_swapchain(c), cmd->image_index, 1, &cmd->cmd_buffer);
			Vulkan::get_swapchain(c)->cmd_buffer_resource_semaphore.release();
			v_free(cmd);
		}
		void wait_idle(RenderContext *c) { wait_idle(Vulkan::get_device(c)); }

		void destroy_render_context(RenderContext *c)
		{
			// Make sure don't destroy anything that is in use.
			wait_idle(c);
			destroy_texture(c, c->normal_texture);
			destroy_texture(c, c->black_texture);
			destroy_texture(c, c->white_texture);
			destroy_swapchain(Vulkan::get_swapchain(c));
		}
	} // namespace Platform
} // namespace Vultr
