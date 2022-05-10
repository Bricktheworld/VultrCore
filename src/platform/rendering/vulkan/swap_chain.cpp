#include "swap_chain.h"
#include "render_context.h"
#include <math/clamp.h>

namespace Vultr
{
	namespace Vulkan
	{
		static VkExtent2D pick_swap_extent(const Platform::Window *window, const VkSurfaceCapabilitiesKHR &capabilities)
		{
			if (capabilities.currentExtent.width != U32Max)
			{
				return capabilities.currentExtent;
			}
			else
			{
				int width, height;
				glfwGetFramebufferSize(static_cast<GLFWwindow *>(Platform::get_window_implementation(window)), &width, &height);

				VkExtent2D extent = {static_cast<u32>(width), static_cast<u32>(height)};

				extent.width      = Vultr::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				extent.height     = Vultr::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return extent;
			}
		}

		static VkSurfaceFormatKHR pick_swap_chain_surface_format(const Vector<VkSurfaceFormatKHR> &available_formats)
		{
			for (const auto &format : available_formats)
			{
				if (format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					{
						return format;
					}
				}
			}

			THROW("No good swap chain format found!");
		}

		static VkPresentModeKHR pick_present_mode(const Vector<VkPresentModeKHR> &available_modes)
		{
			for (const auto &mode : available_modes)
			{
				// We are disabling vsync for now cause it's annoying
				//				if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				//				{
				//					return mode;
				//				}
				if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					return mode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		static void init_swap_chain(SwapChain *sc, const Platform::Window *window)
		{
			auto *d             = &sc->device;
			auto details        = query_swap_chain_support(d);

			auto surface_format = pick_swap_chain_surface_format(details.formats);
			auto present_mode   = pick_present_mode(details.present_modes);
			auto extent         = pick_swap_extent(window, details.capabilities);

			u32 image_count     = details.capabilities.minImageCount + 1;
			if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount)
			{
				image_count = details.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR create_info{
				.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.surface          = d->surface,
				.minImageCount    = image_count,
				.imageFormat      = surface_format.format,
				.imageColorSpace  = surface_format.colorSpace,
				.imageExtent      = extent,
				.imageArrayLayers = 1,
				.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			};

			auto indices               = find_queue_families(d);
			u32 queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

			if (indices.graphics_family != indices.present_family)
			{
				create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
				create_info.queueFamilyIndexCount = 2;
				create_info.pQueueFamilyIndices   = queue_family_indices;
			}
			else
			{
				create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
				create_info.queueFamilyIndexCount = 0;
				create_info.pQueueFamilyIndices   = nullptr;
			}

			create_info.preTransform   = details.capabilities.currentTransform;
			create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			create_info.presentMode    = present_mode;
			create_info.clipped        = VK_TRUE;
			create_info.oldSwapchain   = VK_NULL_HANDLE;

			VK_CHECK(vkCreateSwapchainKHR(d->device, &create_info, nullptr, &sc->swap_chain));
			vkGetSwapchainImagesKHR(d->device, sc->swap_chain, &image_count, nullptr);

			sc->images.resize(image_count);
			vkGetSwapchainImagesKHR(d->device, sc->swap_chain, &image_count, &sc->images[0]);

			sc->image_format = surface_format.format;
			sc->extent       = extent;
		}

		static void init_image_views(SwapChain *sc)
		{
			auto *d = &sc->device;
			sc->image_views.resize(sc->images.size());

			for (size_t i = 0; i < sc->image_views.size(); i++)
			{
				VkImageViewCreateInfo create_info{
					.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.image    = sc->images[i],
					.viewType = VK_IMAGE_VIEW_TYPE_2D,
					.format   = sc->image_format,
					.components =
						{
							.r = VK_COMPONENT_SWIZZLE_IDENTITY,
							.g = VK_COMPONENT_SWIZZLE_IDENTITY,
							.b = VK_COMPONENT_SWIZZLE_IDENTITY,
							.a = VK_COMPONENT_SWIZZLE_IDENTITY,
						},
					.subresourceRange =
						{
							.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel   = 0,
							.levelCount     = 1,
							.baseArrayLayer = 0,
							.layerCount     = 1,
						},
				};
				VK_CHECK(vkCreateImageView(d->device, &create_info, nullptr, &sc->image_views[i]));
			}
		}

		static void init_render_pass(SwapChain *sc)
		{
			auto *d = &sc->device;
			VkAttachmentDescription color_attachment{
				.format         = sc->image_format,
				.samples        = VK_SAMPLE_COUNT_1_BIT,
				.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			};

			VkAttachmentReference color_attachment_ref{
				.attachment = 0,
				.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};

			VkSubpassDescription subpass{
				.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments    = &color_attachment_ref,
			};

			VkSubpassDependency dependency{
				.srcSubpass    = VK_SUBPASS_EXTERNAL,
				.dstSubpass    = 0,
				.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			};

			VkRenderPassCreateInfo render_pass_info{
				.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.attachmentCount = 1,
				.pAttachments    = &color_attachment,
				.subpassCount    = 1,
				.pSubpasses      = &subpass,
				.dependencyCount = 1,
				.pDependencies   = &dependency,
			};

			VK_CHECK(vkCreateRenderPass(d->device, &render_pass_info, nullptr, &sc->render_pass));
		}

		static void init_framebuffers(SwapChain *sc)
		{
			auto *d = &sc->device;
			sc->framebuffers.resize(sc->image_views.size());
			for (size_t i = 0; i < sc->image_views.size(); i++)
			{
				VkImageView attachments[] = {sc->image_views[i]};

				VkFramebufferCreateInfo framebuffer_info{
					.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.renderPass      = sc->render_pass,
					.attachmentCount = 1,
					.pAttachments    = attachments,
					.width           = sc->extent.width,
					.height          = sc->extent.height,
					.layers          = 1,
				};

				VK_CHECK(vkCreateFramebuffer(d->device, &framebuffer_info, nullptr, &sc->framebuffers[i]));
			}
		}

		static void init_frames(SwapChain *sc)
		{
			auto *d = Vulkan::get_device(sc);
			// TODO(Brandon): Don't hard code this stuff, this is very hacky.
			VkDescriptorSetLayoutBinding layout_bindings[] = {
				{
					.binding         = 0,
					.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
				},
				{
					.binding         = 1,
					.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
				},
			};

			VkDescriptorSetLayoutCreateInfo layout_info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext        = nullptr,
				.flags        = 0,
				.bindingCount = static_cast<u32>(2),
				.pBindings    = layout_bindings,
			};
			VK_CHECK(vkCreateDescriptorSetLayout(d->device, &layout_info, nullptr, &sc->default_descriptor_set_layout));

			{
				auto padded_size   = pad_size(d, sizeof(Platform::CameraUBO));
				auto buffer        = alloc_buffer(d, padded_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				void *mapped       = map_buffer(d, &buffer);
				sc->camera_binding = {.buffer = buffer, .mapped = mapped, .updated = {}};
			}
			{
				auto padded_size = pad_size(d, sizeof(Platform::DirectionalLightUBO));
				auto buffer      = alloc_buffer(d, padded_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				void *mapped     = map_buffer(d, &buffer);
				sc->directional_light_binding = {.buffer = buffer, .mapped = mapped, .updated = {}};
			}

			for (u32 i = 0; i < sc->images.size(); i++)
			{
				VkDescriptorPoolSize pool_sizes[] = {{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 2}};

				VkDescriptorPoolCreateInfo pool_info{
					.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
					.flags         = 0,
					.maxSets       = 1,
					.poolSizeCount = 1,
					.pPoolSizes    = pool_sizes,
				};
				VkDescriptorPool pool;
				VK_CHECK(vkCreateDescriptorPool(d->device, &pool_info, nullptr, &pool));

				VkDescriptorSetAllocateInfo info{
					.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
					.pNext              = nullptr,
					.descriptorPool     = pool,
					.descriptorSetCount = 1,
					.pSetLayouts        = &sc->default_descriptor_set_layout,
				};

				VkDescriptorSet vk_set;
				VK_CHECK(vkAllocateDescriptorSets(d->device, &info, &vk_set));

				sc->frames.push_back({
					.cmd_pool                        = init_cmd_pool(get_device(sc)),
					.default_uniform_descriptor_pool = pool,
					.default_uniform_descriptor      = vk_set,
				});
			}
		}

		static void init_in_flight_fence(Device *d, InFlightFence *out)
		{
			VkFenceCreateInfo fence_info{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT,
			};
			VK_CHECK(vkCreateFence(d->device, &fence_info, nullptr, &out->vk_fence));
		}

		static void destroy_in_flight_fence(Device *d, InFlightFence *fence) { vkDestroyFence(d->device, fence->vk_fence, nullptr); }

		static void clear_in_flight_fence(Device *d, InFlightFence *fence)
		{
			Platform::Lock lock(fence->mutex);
			fence->in_use_resources.clear();
		}

		static void init_concurrency(SwapChain *sc)
		{
			auto *d = &sc->device;
			sc->images_in_flight.resize(sc->images.size());
			memset(&sc->images_in_flight[0], 0, sc->images.size() * sizeof(VkFence));

			VkSemaphoreCreateInfo semaphore_info{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

			for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				VK_CHECK(vkCreateSemaphore(d->device, &semaphore_info, nullptr, &sc->image_available_semaphores[i]));
				VK_CHECK(vkCreateSemaphore(d->device, &semaphore_info, nullptr, &sc->render_finished_semaphores[i]));
				init_in_flight_fence(d, &sc->in_flight_fences[i]);
			}
		}

		void init_swapchain(Device *device, const Platform::Window *window, SwapChain *out)
		{
			out->device = *device;
			init_swap_chain(out, window);
			init_image_views(out);
			init_render_pass(out);
			init_framebuffers(out);
			init_frames(out);
			init_concurrency(out);
		}

		static void internal_destroy_swapchain(SwapChain *sc)
		{
			auto *d = &sc->device;

			for (auto *framebuffer : sc->framebuffers)
			{
				vkDestroyFramebuffer(d->device, framebuffer, nullptr);
			}

			vkDestroyRenderPass(d->device, sc->render_pass, nullptr);

			for (auto *view : sc->image_views)
			{
				vkDestroyImageView(d->device, view, nullptr);
			}

			vkDestroySwapchainKHR(d->device, sc->swap_chain, nullptr);
		}

		void destroy_swapchain(SwapChain *sc)
		{
			auto *d = &sc->device;

			for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				vkDestroySemaphore(d->device, sc->render_finished_semaphores[i], nullptr);
				vkDestroySemaphore(d->device, sc->image_available_semaphores[i], nullptr);
				destroy_in_flight_fence(d, &sc->in_flight_fences[i]);
			}

			for (auto &frame : sc->frames)
			{
				vkDestroyDescriptorPool(d->device, frame.default_uniform_descriptor_pool, nullptr);
				destroy_cmd_pool(d, &frame.cmd_pool);
			}

			vkDestroyDescriptorSetLayout(d->device, sc->default_descriptor_set_layout, nullptr);
			unmap_buffer(d, &sc->camera_binding.buffer);
			free_buffer(sc, &sc->camera_binding.buffer);
			unmap_buffer(d, &sc->directional_light_binding.buffer);
			free_buffer(sc, &sc->directional_light_binding.buffer);

			internal_destroy_swapchain(sc);

			destroy_device(d);
		}

		void recreate_swapchain(SwapChain *sc, const Platform::Window *window)
		{
			auto *d = &sc->device;
			// Minimization
			int width = 0, height = 0;

			auto *glfw = static_cast<GLFWwindow *>(Platform::get_window_implementation(window));
			glfwGetFramebufferSize(glfw, &width, &height);
			while (width == 0 || height == 0)
			{
				glfwGetFramebufferSize(glfw, &width, &height);
				glfwWaitEvents();
			}

			wait_idle(d);

			internal_destroy_swapchain(sc);

			init_swap_chain(sc, window);
			init_image_views(sc);
			init_render_pass(sc);
			init_framebuffers(sc);
		}

		ErrorOr<Tuple<u32, Frame *, VkFramebuffer>> acquire_swapchain(SwapChain *sc)
		{
			auto *d = &sc->device;
			vkWaitForFences(d->device, 1, &sc->in_flight_fences[sc->current_frame].vk_fence, VK_TRUE, UINT64_MAX);
			clear_in_flight_fence(d, &sc->in_flight_fences[sc->current_frame]);

			u32 image_index;
			auto res = vkAcquireNextImageKHR(d->device, sc->swap_chain, U64Max, sc->image_available_semaphores[sc->current_frame], VK_NULL_HANDLE, &image_index);
			if (res == VK_ERROR_OUT_OF_DATE_KHR)
			{
				// If this occurs, then try again next draw frame call.
				return Error("Need to recreate swap chain.");
			}
			else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
			{
				THROW("Failed to acquire swap chain images!");
			}
			ASSERT(image_index != 256, "Image index is 256??");

			if (sc->images_in_flight[image_index] != VK_NULL_HANDLE)
			{
				VK_CHECK(vkWaitForFences(d->device, 1, &sc->images_in_flight[image_index], VK_TRUE, U64Max));
			}

			sc->images_in_flight[image_index] = sc->in_flight_fences[sc->current_frame].vk_fence;
			return Tuple(image_index, &sc->frames[image_index], sc->framebuffers[image_index]);
		}

		ErrorOr<void> submit_swapchain(SwapChain *sc, u32 image_index, u32 command_buffer_count, VkCommandBuffer *command_buffers)
		{
			auto *d                            = &sc->device;

			VkSemaphore wait_semaphores[]      = {sc->image_available_semaphores[sc->current_frame]};
			VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
			VkSemaphore signal_semaphores[]    = {sc->render_finished_semaphores[sc->current_frame]};

			VkSubmitInfo submit_info{
				.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.waitSemaphoreCount   = 1,
				.pWaitSemaphores      = wait_semaphores,
				.pWaitDstStageMask    = wait_stages,
				.commandBufferCount   = command_buffer_count,
				.pCommandBuffers      = command_buffers,
				.signalSemaphoreCount = 1,
				.pSignalSemaphores    = signal_semaphores,
			};
			VK_CHECK(vkResetFences(d->device, 1, &sc->in_flight_fences[sc->current_frame].vk_fence));
			graphics_queue_submit(d, 1, &submit_info, sc->in_flight_fences[sc->current_frame].vk_fence);

			VkPresentInfoKHR present_info{
				.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores    = signal_semaphores,
				.swapchainCount     = 1,
				.pSwapchains        = &sc->swap_chain,
				.pImageIndices      = &image_index,
				.pResults           = nullptr,
			};

			sc->current_frame = (sc->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

			auto res          = vkQueuePresentKHR(d->present_queue, &present_info);
			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || sc->framebuffer_was_resized)
			{
				sc->framebuffer_was_resized = false;
				return Error("Need to recreate swap chain!");
			}
			else if (res != VK_SUCCESS)
			{
				THROW("Failed to present swap chain images!");
			}
			return None;
		}

		ErrorOr<void> queue_cmd_buffer(SwapChain *sc, VkCommandBuffer command_buffer, VkFence fence)
		{
			auto *d = &sc->device;

			VkSubmitInfo submit_info{
				.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.commandBufferCount = 1,
				.pCommandBuffers    = &command_buffer,
			};

			if (fence != nullptr)
				VK_CHECK(vkResetFences(d->device, 1, &fence));
			Platform::Lock lock(d->graphics_queue_mutex);
			VK_TRY(vkQueueSubmit(d->graphics_queue, 1, &submit_info, fence));
			return Success;
		}

		ErrorOr<void> wait_queue_cmd_buffer(SwapChain *sc, VkCommandBuffer command_buffer)
		{
			TRY(queue_cmd_buffer(sc, command_buffer));
			vkQueueWaitIdle(sc->device.graphics_queue);
			return Success;
		}

		void wait_resource_not_in_use(SwapChain *sc, void *resource)
		{
			Platform::Lock lock(sc->cmd_buffer_resource_mutex);
			VkFence fences[Vulkan::MAX_FRAMES_IN_FLIGHT]{};
			u32 count = 0;
			for (auto &fence : sc->in_flight_fences)
			{
				fence.mutex.lock();
				if (fence.in_use_resources.contains(resource))
				{
					count++;
					fences[count - 1] = fence.vk_fence;
				}
			}

			if (count > 0)
			{
				auto *d = Vulkan::get_device(sc);
				VK_CHECK(vkWaitForFences(d->device, count, fences, VK_TRUE, U64Max));
			}

			for (auto &fence : sc->in_flight_fences)
			{
				fence.mutex.unlock();
			}
		}

	} // namespace Vulkan
	namespace Platform
	{
		void framebuffer_resize_callback(const Platform::Window *window, Platform::RenderContext *c, u32 width, u32 height) { c->swap_chain.framebuffer_was_resized = true; }

		static void update_descriptor_set(CmdBuffer *cmd, Vulkan::DefaultDescriptorBinding *binding, void *data, size_t size)
		{
			auto *d     = Vulkan::get_device(cmd->render_context);
			auto *sc    = Vulkan::get_swapchain(cmd->render_context);
			void *start = binding->mapped;

			if (memcmp(data, start, size) != 0)
			{
				binding->updated.set_all();
				memcpy(start, data, size);
			}

			if (binding->updated.at(cmd->image_index))
			{
				VkDescriptorBufferInfo camera_info{
					.buffer = sc->camera_binding.buffer.buffer,
					.offset = 0,
					.range  = pad_size(d, sizeof(Platform::CameraUBO)),
				};
				auto set = cmd->frame->default_uniform_descriptor;
				VkWriteDescriptorSet write_sets[2];
				write_sets[0] = {
					.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext           = nullptr,
					.dstSet          = set,
					.dstBinding      = 0,
					.descriptorCount = 1,
					.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pBufferInfo     = &camera_info,
				};
				VkDescriptorBufferInfo directional_light_info{
					.buffer = sc->directional_light_binding.buffer.buffer,
					.offset = 0,
					.range  = pad_size(d, sizeof(Platform::DirectionalLightUBO)),
				};
				write_sets[1] = {
					.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext           = nullptr,
					.dstSet          = set,
					.dstBinding      = 1,
					.descriptorCount = 1,
					.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pBufferInfo     = &directional_light_info,
				};
				vkUpdateDescriptorSets(d->device, 2, write_sets, 0, nullptr);
				binding->updated.set(cmd->image_index, false);
			}
		}

		void update_default_descriptor_set(CmdBuffer *cmd, CameraUBO *camera_ubo, DirectionalLightUBO *directional_light_ubo)
		{
			auto *sc = Vulkan::get_swapchain(cmd->render_context);
			update_descriptor_set(cmd, &sc->camera_binding, camera_ubo, sizeof(CameraUBO));
			update_descriptor_set(cmd, &sc->directional_light_binding, directional_light_ubo, sizeof(DirectionalLightUBO));
		}
	} // namespace Platform
} // namespace Vultr
