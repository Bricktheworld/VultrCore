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
		struct SwapChainImage
		{
			VkImage image{};
			VkImageView image_view{};
			VkFramebuffer framebuffer{};

			// A reference to a frame completed fence, since when we are beginning another frame in flight we might acquire an image that is still in flight.
			Option<VkFence *> available_fence = None;
		};

		struct Frame
		{
			// A pool for each frame where you can allocate descriptor sets and command buffers.
			CommandPool cmd_pool{};
			VkDescriptorPool default_uniform_descriptor_pool{};
			VkDescriptorSet default_uniform_descriptor{};

			// CPU fence to signal when graphics queue has fully processed this frame.
			VkFence completed_fence = nullptr;
			// GPU semaphore to signal when the vkAcquireNextImageKHR has officially acquired an image.
			VkSemaphore acquire_sem = nullptr;
			// GPU semaphore to signal when it is okay to present this frame.
			VkSemaphore present_sem = nullptr;

			// A hashtable of resources that are in use by a given frame.
			HashTable<void *, Traits<void *>> in_use_resources{};
			Platform::Mutex mutex{};
		};

		struct DefaultDescriptorBinding
		{
			Vulkan::GpuBuffer buffer{};
			void *mapped         = nullptr;
			Bitfield<64> updated = {};
		};

		struct AcquiredSwapchainFrame
		{
			Frame *frame                     = nullptr;
			VkFramebuffer output_framebuffer = nullptr;
			u32 image_index                  = 0;
		};

		struct SwapChain
		{
			VkSwapchainKHR swap_chain = nullptr;
			VkFormat image_format{};
			VkExtent2D extent{};
			VkRenderPass render_pass = nullptr;
			Device device{};
			Vector<SwapChainImage> images{};
			Frame frames[Vulkan::MAX_FRAMES_IN_FLIGHT];
			u32 current_frame            = 0;

			bool framebuffer_was_resized = false;

			VkDescriptorSetLayout default_descriptor_set_layout{};
			DefaultDescriptorBinding camera_binding{};
			DefaultDescriptorBinding directional_light_binding{};
			Platform::Mutex cmd_buffer_resource_mutex{};
		};

		void init_swapchain(Device *device, const Platform::Window *window, SwapChain *out);
		void recreate_swapchain(SwapChain *sc, const Platform::Window *window);
		ErrorOr<AcquiredSwapchainFrame> acquire_swapchain(SwapChain *sc);
		ErrorOr<void> submit_swapchain(SwapChain *sc, u32 image_index, u32 command_buffer_count, VkCommandBuffer *command_buffers);
		ErrorOr<void> queue_cmd_buffer(SwapChain *sc, VkCommandBuffer command_buffer, VkFence fence = VK_NULL_HANDLE);
		ErrorOr<void> wait_queue_cmd_buffer(SwapChain *sc, VkCommandBuffer command_buffer);
		void wait_resource_not_in_use(SwapChain *sc, void *resource);
		void destroy_swapchain(SwapChain *sc);
	}; // namespace Vulkan
} // namespace Vultr
