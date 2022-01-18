#include "render_context.h"

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

			c->device = Vulkan::init_device(window, debug, Vulkan::debug_cb);
			//			init_render_pass(c);
			//			init_graphics_pipeline(c);
			//			init_framebuffers(c);
			//			init_command_pools(c);
			//			init_vertex_buffer(c);
			//			init_index_buffer(c);
			//			init_command_buffers(c);
			//			init_concurrency(c);
			return c;
		}

		void framebuffer_resize_callback(const Platform::Window *window, RenderContext *c, u32 width, u32 height) { c->framebuffer_was_resized = true; }

		void draw_frame(const Platform::Window *window, RenderContext *c) {}

		void destroy_render_context(RenderContext *c)
		{
			auto *device = &c->device;
			// Make sure don't destroy anything that is in use.
			wait_idle(device);

			//			vkDestroyBuffer(device, c->index_buffer, nullptr);
			//			vkFreeMemory(device, c->index_buffer_memory, nullptr);
			//
			//			vkDestroyBuffer(device, c->vertex_buffer, nullptr);
			//			vkFreeMemory(device, c->vertex_buffer_memory, nullptr);
			//
			//			for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			//			{
			//				vkDestroySemaphore(device, c->render_finished_semaphores[i], nullptr);
			//				vkDestroySemaphore(device, c->image_available_semaphores[i], nullptr);
			//				vkDestroyFence(device, c->in_flight_fences[i], nullptr);
			//			}
			destroy_device(device);
		}
	} // namespace Platform
} // namespace Vultr