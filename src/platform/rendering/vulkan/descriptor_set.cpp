#include "descriptor_set.h"
#include "shader.h"
#include "texture.h"

namespace Vultr
{
	namespace Vulkan
	{
		size_t pad_size(Device *d, size_t size)
		{
			auto alignment    = Vulkan::min_ubo_alignment(d);
			auto aligned_size = size;
			if (alignment > 0)
			{
				aligned_size = (aligned_size + alignment - 1) & ~(alignment - 1);
			}
			return aligned_size;
		}

		VkDescriptorPool init_descriptor_pool(Device *d)
		{
			VkDescriptorPoolSize pool_sizes[] = {
				{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 256},
				{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1024},
			};

			VkDescriptorPoolCreateInfo info{
				.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.flags         = 0,
				.maxSets       = 128,
				.poolSizeCount = 2,
				.pPoolSizes    = pool_sizes,
			};
			VkDescriptorPool pool = nullptr;
			VK_CHECK(vkCreateDescriptorPool(d->device, &info, nullptr, &pool));
			return pool;
		}
		void destroy_descriptor_pool(Device *d, VkDescriptorPool descriptor_pool) { vkDestroyDescriptorPool(d->device, descriptor_pool, nullptr); }

	} // namespace Vulkan
	namespace Platform
	{
		void update_descriptor_set(DescriptorSet *set, void *data, size_t size)
		{
			ASSERT(data != nullptr, "Cannot update descriptor set with nullptr buffer!");

			auto *start = set->uniform_buffer_bindings[0].mapped;

			if (memcmp(data, start, size) == 0)
				return;

			memcpy(start, data, size);
			set->updated.set_all();
		}

		void update_descriptor_set(DescriptorSet *set, Texture *texture, u32 binding)
		{
			u64 hash = 0;
			if (texture == nullptr)
			{
				if (set->image_bindings[binding - 1].get<1>() == nullptr)
					return;
			}
			else
			{
				hash = Traits<Texture>::hash(*texture);
				if (set->image_bindings[binding - 1].get<0>() == hash)
					return;
			}

			set->image_bindings[binding - 1] = {hash, texture};
			set->updated.set_all();
		}

		void bind_descriptor_set(CmdBuffer *cmd, Shader *shader, DescriptorSet *set, const GraphicsPipelineInfo &info, const Option<PushConstant> &constant)
		{
			auto vk_set    = set->vk_frame_descriptor_sets[cmd->frame_index];

			auto *pipeline = Vulkan::get_or_create_pipeline(cmd->render_context, shader, info, cmd->current_framebuffer);
			Vulkan::bind_pipeline(cmd, pipeline);

			if (constant)
				vkCmdPushConstants(cmd->cmd_buffer, pipeline->vk_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &constant.value());

			vkCmdBindDescriptorSets(cmd->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_layout, 1, 1, &vk_set, 0, nullptr);

			Vulkan::depend_resource(cmd, shader);
			Vulkan::depend_resource(cmd, set);
			Vulkan::depend_resource(cmd, &set->uniform_buffer_bindings[0].buffer);

			for (auto [_, image] : set->image_bindings)
			{
				if (image != nullptr)
					Vulkan::depend_resource(cmd, image);
			}
		}
	} // namespace Platform
} // namespace Vultr
