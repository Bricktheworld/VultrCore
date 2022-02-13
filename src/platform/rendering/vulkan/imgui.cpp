#include "../../platform_impl.h"
#include "render_context.h"
#include <types/array.h>
#include <vulkan/vulkan.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui_impl_glfw.h>

namespace Vultr
{
	namespace Platform
	{
		void init_imgui(const Window *window, RenderContext *c)
		{
			auto pool_sizes                      = Array<VkDescriptorPoolSize, 11>({{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                                               {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                                               {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                                               {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                                               {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                                               {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                                               {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                                               {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                                               {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                                               {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                                               {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}});

			VkDescriptorPoolCreateInfo pool_info = {
				.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
				.maxSets       = 1000,
				.poolSizeCount = pool_sizes.size(),
				.pPoolSizes    = &pool_sizes[0],
			};

			VkDescriptorPool imgui_pool;
			VK_CHECK(vkCreateDescriptorPool(c->swap_chain.device.device, &pool_info, nullptr, &imgui_pool));

			ImGui::CreateContext();

			auto *glfw = static_cast<GLFWwindow *>(Platform::get_window_implementation(window));
			ImGui_ImplGlfw_InitForVulkan(glfw, true);

			ImGui_ImplVulkan_InitInfo init_info = {
				.Instance       = c->swap_chain.device.instance,
				.PhysicalDevice = c->swap_chain.device.physical_device,
				.Device         = c->swap_chain.device.device,
				.Queue          = c->swap_chain.device.graphics_queue,
				.DescriptorPool = imgui_pool,
				.MinImageCount  = 3,
				.ImageCount     = 3,
				.MSAASamples    = VK_SAMPLE_COUNT_1_BIT,

			};
			ImGui_ImplVulkan_Init(&init_info, c->swap_chain.render_pass);
			//			ImGui_ImplVulkan_CreateFontsTexture(cmd);

			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		void destroy_imgui(const Window *window, RenderContext *c) { vkDestroyDescriptorPool(c->swap_chain.device.device, imguiPool, nullptr); }
	} // namespace Platform
} // namespace Vultr
