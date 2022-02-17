#include "descriptor_set.h"

namespace Vultr
{
	static size_t pad_size(Vulkan::Device *d, size_t size)
	{
		auto alignment    = Vulkan::min_ubo_alignment(d);
		auto aligned_size = size;
		if (alignment > 0)
		{
			aligned_size = (aligned_size + alignment - 1) & ~(alignment - 1);
		}
		return aligned_size;
	}

	namespace Vulkan
	{
		VkDescriptorPool init_descriptor_pool(Device *d)
		{
			VkDescriptorPoolSize pool_sizes[] = {
				{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount = 100},
			};

			VkDescriptorPoolCreateInfo info{
				.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.flags         = 0,
				.maxSets       = 10,
				.poolSizeCount = 1,
				.pPoolSizes    = pool_sizes,
			};
			VkDescriptorPool pool = nullptr;
			VK_CHECK(vkCreateDescriptorPool(d->device, &info, nullptr, &pool));
			return pool;
		}
		void destroy_descriptor_pool(Device *d, VkDescriptorPool descriptor_pool) { vkDestroyDescriptorPool(d->device, descriptor_pool, nullptr); }

		DescriptorSetBuffer init_descriptor_set_buffer(Device *d, VkDescriptorPool pool, Platform::DescriptorLayout *layout)
		{
			auto padded_size = pad_size(d, layout->size);
			DescriptorSetBuffer descriptor_buffer{
				.layout = layout,
				.buffer =
					alloc_buffer(d, padded_size * layout->max_objects, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
			};
			descriptor_buffer.mapped = map_buffer(d, &descriptor_buffer.buffer);

			VkDescriptorSetAllocateInfo info{
				.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.descriptorPool     = pool,
				.descriptorSetCount = 1,
				.pSetLayouts        = &layout->vk_layout,
				.pNext              = nullptr,
			};

			VK_CHECK(vkAllocateDescriptorSets(d->device, &info, &descriptor_buffer.vk_set));
			return descriptor_buffer;
		}

		void destroy_descriptor_set_buffer(Device *d, DescriptorSetBuffer *set_buffer)
		{
			unmap_buffer(d, &set_buffer->buffer);
			free_buffer(d, &set_buffer->buffer);
		}

	} // namespace Vulkan
	namespace Platform
	{
		DescriptorLayout *init_descriptor_layout(RenderContext *c, size_t size, u32 max_objects)
		{
			ASSERT(max_objects > 0, "Max descriptor objects should be greater than 0!");
			auto *layout        = v_alloc<DescriptorLayout>();
			layout->size        = size;
			layout->max_objects = max_objects;
			VkDescriptorSetLayoutBinding layout_binding{
				.binding         = 0,
				.descriptorCount = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
				.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			};

			VkDescriptorSetLayoutCreateInfo info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext        = nullptr,
				.bindingCount = 1,
				.flags        = 0,
				.pBindings    = &layout_binding,
			};

			VK_CHECK(vkCreateDescriptorSetLayout(Vulkan::get_device(c)->device, &info, nullptr, &layout->vk_layout));
			return layout;
		}

		void destroy_descriptor_layout(RenderContext *c, DescriptorLayout *layout)
		{
			vkDestroyDescriptorSetLayout(Vulkan::get_device(c)->device, layout->vk_layout, nullptr);
			layout->vk_layout = nullptr;
			v_free(layout);
		}

		void update_descriptor_set(CmdBuffer *cmd, DescriptorLayout *layout, void *data, u32 index)
		{
			ASSERT(index < layout->max_objects, "Index %u is greater than the max number of objects %u in descriptor layout!", index, layout->max_objects);
			auto *c              = cmd->render_context;
			auto *d              = Vulkan::get_device(c);
			auto *frame          = cmd->frame;

			auto *descriptor_set = &frame->descriptor_buffers.get(layout);

			auto padded_size     = pad_size(d, layout->size);
			void *start          = reinterpret_cast<byte *>(descriptor_set->mapped) + index * padded_size;

			if (memcmp(data, start, layout->size) == 0)
				return;

			memcpy(start, data, layout->size);
			descriptor_set->updated = true;
		}

		void flush_descriptor_set_changes(CmdBuffer *cmd)
		{
			auto *c     = cmd->render_context;
			auto *frame = cmd->frame;
			Vector<VkWriteDescriptorSet> write_sets{};
			Vector<VkDescriptorBufferInfo> buffer_infos{};
			u32 i = 0;
			for (auto [layout, descriptor_set] : frame->descriptor_buffers)
			{
				auto &info = buffer_infos.push_back({
					.buffer = descriptor_set.buffer.buffer,
					.offset = 0,
					.range  = pad_size(Vulkan::get_device(c), layout->size) * layout->max_objects,
				});
				write_sets.push_back({
					.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext           = nullptr,
					.dstSet          = descriptor_set.vk_set,
					.dstBinding      = 0,
					.descriptorCount = 1,
					.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
					.pBufferInfo     = &info,
				});
				i++;
			}
			vkUpdateDescriptorSets(Vulkan::get_device(c)->device, write_sets.size(), &write_sets[0], 0, nullptr);
		}

		void bind_descriptor_set(CmdBuffer *cmd, GraphicsPipeline *pipeline, DescriptorLayout *layout, u32 set, u32 index)
		{
			auto *c              = cmd->render_context;
			auto *frame          = cmd->frame;
			auto *descriptor_set = &frame->descriptor_buffers.get(layout);
			u32 offset           = pad_size(Vulkan::get_device(c), layout->size) * index;
			vkCmdBindDescriptorSets(cmd->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_layout, set, 1, &descriptor_set->vk_set, 1, &offset);
		}
	} // namespace Platform
} // namespace Vultr