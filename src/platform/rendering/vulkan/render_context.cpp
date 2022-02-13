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

		//		static void init_descriptor_set_layout(Platform::RenderContext *c)
		//		{
		//			auto *d = &c->swap_chain.device;
		//			VkDescriptorSetLayoutBinding ubo_layout_binding{
		//				.binding            = 0,
		//				.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		//				.descriptorCount    = 1,
		//				.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
		//				.pImmutableSamplers = nullptr,
		//			};
		//			VkDescriptorSetLayoutCreateInfo layout_info{
		//				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		//				.bindingCount = 1,
		//				.pBindings    = &ubo_layout_binding,
		//			};
		//			VK_CHECK(vkCreateDescriptorSetLayout(d->device, &layout_info, nullptr, &c->descriptor_set_layout));
		//		}

		// TODO(Brandon): Definitely don't do this here.
		static void init_graphics_pipeline(Platform::RenderContext *c)
		{
			auto *d = &c->swap_chain.device;
			Buffer vert_shader_src;
			fread_all(Path("./build/shaders/basic_vert.spv"), &vert_shader_src);

			Buffer frag_shader_src;
			fread_all(Path("./build/shaders/basic_frag.spv"), &frag_shader_src);

			// auto vert_shader_module     = init_shader_module(c, vert_shader_src);
			// auto frag_shader_module     = init_shader_module(c, frag_shader_src);

			// auto binding_description    = get_binding_description();
			// auto attribute_descriptions = get_attribute_descriptions();

			// vkDestroyShaderModule(d->device, frag_shader_module, nullptr);
			// vkDestroyShaderModule(d->device, vert_shader_module, nullptr);
		}

		//		static void update_uniform_buffer(Platform::RenderContext *c, u32 image_index, f64 dt)
		//		{
		//			auto *sc = &c->swap_chain;
		//			auto *d  = &sc->device;
		//			c->rotation += 1 * dt;
		//
		//			auto extent = sc->extent;
		//
		//			UniformBufferObject ubo{
		//				.model = glm::rotate(glm::mat4(1.0f), glm::radians((f32)c->rotation), glm::vec3(0.0f, 0.0f, 1.0f)),
		//				.view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		//				.proj  = glm::perspective(glm::radians(45.0f), (f32)extent.width / (f32)extent.height, 0.1f, 10.0f),
		//			};
		//			ubo.proj[1][1] *= -1;
		//
		//			void *data;
		//			vkMapMemory(d->device, sc->uniform_buffers[image_index].memory, 0, sizeof(ubo), 0, &data);
		//			memcpy(data, &ubo, sizeof(ubo));
		//			vkUnmapMemory(d->device, sc->uniform_buffers[image_index].memory);
		//		}

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
				auto [image_index, cmd_pool, framebuffer] = res.value();
				//				Vulkan::update_uniform_buffer(c, image_index, dt);

				auto cmd                 = Vulkan::begin_cmd_buffer(Vulkan::get_device(c), cmd_pool);

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

				// vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, c->graphics_pipeline);

				// VkBuffer vertex_buffers[] = {c->vertex_buffer.buffer};
				// VkDeviceSize offsets[]    = {0};
				// vkCmdBindVertexBuffers(cmd, 0, 1, vertex_buffers, offsets);
				// vkCmdBindIndexBuffer(cmd, c->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);

				// vkCmdDrawIndexed(cmd, static_cast<u32>(c->indices.size()), 1, 0, 0, 0);

				auto *buf           = v_alloc<CmdBuffer>();
				buf->render_context = c;
				buf->cmd_buffer     = cmd;
				buf->cmd_pool       = cmd_pool;
				buf->image_index    = image_index;

				return buf;
			}
			else
			{
				auto *d = &c->swap_chain.device;
				Vulkan::recreate_swapchain(&c->swap_chain, window);
				return Error("Need to recreate swap chain.");

				// Vulkan::init_graphics_pipeline(c);
				// Vulkan::init_command_buffers(c);
			}
		}

		void end_cmd_buffer(CmdBuffer *cmd)
		{
			auto *c = cmd->render_context;
			vkCmdEndRenderPass(cmd->cmd_buffer);
			Vulkan::end_cmd_buffer(cmd->cmd_buffer, cmd->cmd_pool);
			Vulkan::submit_swapchain(Vulkan::get_swapchain(c), cmd->image_index, 1, &cmd->cmd_buffer);
			v_free(cmd);
		}

		void destroy_render_context(RenderContext *c)
		{
			auto *d = &c->swap_chain.device;
			// Make sure don't destroy anything that is in use.
			wait_idle(d);

			// Vulkan::free_buffer(d, &c->vertex_buffer);
			// Vulkan::free_buffer(d, &c->index_buffer);

			//			vkDestroyDescriptorSetLayout(d->device, c->descriptor_set_layout, nullptr);

			// vkDestroyPipeline(d->device, c->graphics_pipeline, nullptr);
			// vkDestroyPipelineLayout(d->device, c->pipeline_layout, nullptr);

			destroy_swapchain(&c->swap_chain);
		}
	} // namespace Platform
} // namespace Vultr
