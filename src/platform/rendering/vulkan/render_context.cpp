#include <filesystem/filestream.h>
#include "render_context.h"
#include "../../rendering.h"
#include "gpu_buffer.h"

namespace Vultr
{
	namespace Vulkan
	{
		struct UniformBufferObject
		{
			Mat4 model;
			Mat4 view;
			Mat4 proj;
		};

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

			auto device   = Vulkan::init_device(window, debug, Vulkan::debug_cb);
			c->swap_chain = Vulkan::init_swapchain(device, window);
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
				//				Vulkan::update_uniform_buffer(c, image_index, dt);

				Vulkan::recycle_cmd_pool(Vulkan::get_device(c), cmd_pool);
				auto cmd = Vulkan::begin_cmd_buffer(Vulkan::get_device(c), cmd_pool);

				VkViewport viewport{
					.x        = 0,
					.y        = 0,
					.width    = static_cast<f32>(c->swap_chain.extent.width),
					.height   = static_cast<f32>(c->swap_chain.extent.height),
					.minDepth = 0.0f,
					.maxDepth = 1.0f,
				};

				vkCmdSetViewport(cmd, 0, 1, &viewport);

				VkRect2D scissor{
					.offset = {.x = 0, .y = 0},
					.extent = c->swap_chain.extent,
				};
				vkCmdSetScissor(cmd, 0, 1, &scissor);

				VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

				VkRenderPassBeginInfo render_pass_info{
					.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass  = c->swap_chain.render_pass,
					.framebuffer = framebuffer,
					.renderArea =
						{
							.offset = {0, 0},
							.extent = c->swap_chain.extent,
						},
					.clearValueCount = 1,
					.pClearValues    = &clear_color,
				};

				vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

				auto *buf           = v_alloc<CmdBuffer>();
				buf->render_context = c;
				buf->cmd_buffer     = cmd;
				buf->frame          = frame;
				buf->image_index    = image_index;

				return buf;
			}
			else
			{
				Vulkan::recreate_swapchain(Vulkan::get_swapchain(c), window);
				return Error("Need to recreate swap chain.");
			}
		}

		void end_cmd_buffer(CmdBuffer *cmd)
		{
			auto *c = cmd->render_context;
			vkCmdEndRenderPass(cmd->cmd_buffer);
			Vulkan::end_cmd_buffer(cmd->cmd_buffer, &cmd->frame->cmd_pool);
			Vulkan::submit_swapchain(Vulkan::get_swapchain(c), cmd->image_index, 1, &cmd->cmd_buffer);
			v_free(cmd);
		}
		void wait_idle(RenderContext *c) { wait_idle(Vulkan::get_device(c)); }

		void destroy_render_context(RenderContext *c)
		{
			// Make sure don't destroy anything that is in use.
			wait_idle(c);
			destroy_swapchain(Vulkan::get_swapchain(c));
		}
	} // namespace Platform
} // namespace Vultr
