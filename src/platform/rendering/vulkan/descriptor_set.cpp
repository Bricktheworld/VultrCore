#include "descriptor_set.h"

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
		void update_descriptor_set(DescriptorSet *set, void *data)
		{
			auto *start = set->uniform_buffer_binding.mapped;
			auto size   = set->shader->uniform_size;

			if (memcmp(data, start, size) == 0)
				return;

			memcpy(start, data, size);
			set->updated = U8Max;
		}

		void update_descriptor_set(DescriptorSet *set, const Resource<Platform::Texture *> &texture, u32 binding)
		{
			auto &existing_texture = set->sampler_bindings[binding - 1];
			if (existing_texture.has_value() && texture == existing_texture.value())
				return;

			existing_texture = ResourceId(texture);
			set->updated     = U8Max;
		}

		void bind_descriptor_set(CmdBuffer *cmd, GraphicsPipeline *pipeline, DescriptorSet *set)
		{
			auto vk_set = set->vk_frame_descriptor_sets[cmd->image_index];
			ASSERT(set->shader == pipeline->layout.shader, "Descriptor set is not compatible with pipeline!");
			vkCmdBindDescriptorSets(cmd->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_layout, 1, 1, &vk_set, 0, nullptr);
		}
	} // namespace Platform
} // namespace Vultr