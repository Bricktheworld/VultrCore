#pragma once
#include "device.h"
#include "gpu_buffer.h"
#include "command_pool.h"
#include "descriptor_set.h"
#include <types/tuple.h>
#include <platform/platform.h>

namespace Vultr
{
	namespace Vulkan
	{
		struct Frame
		{
			CommandPool cmd_pool{};
			VkDescriptorPool default_uniform_descriptor_pool{};
			VkDescriptorSet default_uniform_descriptor{};
		};

		struct DefaultDescriptorBinding
		{
			Vulkan::GpuBuffer buffer{};
			void *mapped         = nullptr;
			Bitfield<64> updated = {};
		};

		struct InFlightFence
		{
			VkFence vk_fence = nullptr;
			HashTable<void *, Traits<void *>> in_use_resources{};
			Platform::Mutex mutex{};
			InFlightFence() = default;
			InFlightFence(const InFlightFence &other) : vk_fence(other.vk_fence), in_use_resources(other.in_use_resources) {}

			InFlightFence &operator=(const InFlightFence &other)
			{
				vk_fence         = other.vk_fence;
				in_use_resources = other.in_use_resources;
				return *this;
			}
		};

		struct SwapChain
		{
			VkSwapchainKHR swap_chain = nullptr;
			VkFormat image_format{};
			VkExtent2D extent{};
			Vector<VkImage> images{};
			Vector<VkImageView> image_views{};
			Vector<VkFramebuffer> framebuffers{};
			VkRenderPass render_pass = nullptr;

			VkSemaphore image_available_semaphores[Vulkan::MAX_FRAMES_IN_FLIGHT]{};
			VkSemaphore render_finished_semaphores[Vulkan::MAX_FRAMES_IN_FLIGHT]{};
			InFlightFence in_flight_fences[Vulkan::MAX_FRAMES_IN_FLIGHT]{};
			Vector<VkFence> images_in_flight{};

			u32 current_frame = 0;

			Device device{};
			Vector<Frame> frames{};

			bool framebuffer_was_resized = false;

			VkDescriptorSetLayout default_descriptor_set_layout{};
			DefaultDescriptorBinding camera_binding{};
			DefaultDescriptorBinding directional_light_binding{};
			Platform::Mutex cmd_buffer_resource_mutex{};
		};

		void init_swapchain(Device *device, const Platform::Window *window, SwapChain *out);
		void recreate_swapchain(SwapChain *sc, const Platform::Window *window);
		ErrorOr<Tuple<u32, Frame *, VkFramebuffer>> acquire_swapchain(SwapChain *sc);
		ErrorOr<void> submit_swapchain(SwapChain *sc, u32 image_index, u32 command_buffer_count, VkCommandBuffer *command_buffers);
		ErrorOr<void> queue_cmd_buffer(SwapChain *sc, VkCommandBuffer command_buffer, VkFence fence = VK_NULL_HANDLE);
		ErrorOr<void> wait_queue_cmd_buffer(SwapChain *sc, VkCommandBuffer command_buffer);
		void wait_resource_not_in_use(SwapChain *sc, void *resource);
		void destroy_swapchain(SwapChain *sc);
	}; // namespace Vulkan
} // namespace Vultr
