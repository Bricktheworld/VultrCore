#include "../../platform_impl.h"
#include "render_context.h"
#include "command_pool.h"
#include <types/array.h>
#include <vulkan/vulkan.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui_impl_glfw.h>

namespace Vultr
{
	namespace Platform
	{
		struct ImGuiContext
		{
			VkDescriptorPool descriptor_pool = nullptr;
		};
		ImGuiContext *init_imgui(const Window *window, UploadContext *upload_context)
		{
			auto *imgui_c                        = v_alloc<ImGuiContext>();
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

			auto *c = get_render_context(window);
			auto *d = Vulkan::get_device(c);
			VK_CHECK(vkCreateDescriptorPool(d->device, &pool_info, nullptr, &imgui_c->descriptor_pool));

			ImGui::CreateContext();
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

			auto *glfw = static_cast<GLFWwindow *>(Platform::get_window_implementation(window));
			ImGui_ImplGlfw_InitForVulkan(glfw, true);

			ImGui_ImplVulkan_InitInfo init_info = {
				.Instance       = d->instance,
				.PhysicalDevice = d->physical_device,
				.Device         = d->device,
				.Queue          = d->graphics_queue,
				.DescriptorPool = imgui_c->descriptor_pool,
				.MinImageCount  = 3,
				.ImageCount     = 3,
				.MSAASamples    = VK_SAMPLE_COUNT_1_BIT,
			};
			ImGui_ImplVulkan_Init(&init_info, Vulkan::get_swapchain(c)->render_pass);
			auto cmd = Vulkan::begin_cmd_buffer(d, &upload_context->cmd_pool);
			ImGui_ImplVulkan_CreateFontsTexture(cmd);
			Vulkan::end_cmd_buffer(cmd, &upload_context->cmd_pool);

			Vulkan::wait_queue_cmd_buffer(Vulkan::get_swapchain(c), cmd);

			ImGui_ImplVulkan_DestroyFontUploadObjects();
			return imgui_c;
		}

		void imgui_begin_frame(CmdBuffer *cmd, ImGuiContext *c)
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		void imgui_end_frame(CmdBuffer *cmd, ImGuiContext *c)
		{
			ImGuiIO &io = ImGui::GetIO();
			ImGui::Render();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
			ASSERT(cmd->image_index != 256, "Image index is 256??");
			auto *d = Vulkan::get_device(cmd->render_context);
			Platform::Lock lock(d->graphics_queue_mutex);
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd->cmd_buffer);
		}

		void destroy_imgui(RenderContext *c, ImGuiContext *imgui_c)
		{
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			vkDestroyDescriptorPool(Vulkan::get_device(c)->device, imgui_c->descriptor_pool, nullptr);
			v_free(imgui_c);
		}

		ImTextureID imgui_get_texture_id(Texture *texture)
		{
			if (texture->cached_texture_id == nullptr)
				texture->cached_texture_id = ImGui_ImplVulkan_AddTexture(texture->sampler, texture->image_view, texture->layout);

			return texture->cached_texture_id;
		}

		void imgui_free_texture_id(Texture *texture)
		{
			ASSERT(texture->cached_texture_id != nullptr, "Cannot free invalid ImGui texture ID!");
			ImGui_ImplVulkan_RemoveTexture(texture->cached_texture_id);
		}
	} // namespace Platform
} // namespace Vultr
