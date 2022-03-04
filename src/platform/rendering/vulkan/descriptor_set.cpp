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
				{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount = 1000},
				{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000},
			};

			VkDescriptorPoolCreateInfo info{
				.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.flags         = 0,
				.maxSets       = 10,
				.poolSizeCount = 2,
				.pPoolSizes    = pool_sizes,
			};
			VkDescriptorPool pool = nullptr;
			VK_CHECK(vkCreateDescriptorPool(d->device, &info, nullptr, &pool));
			return pool;
		}
		void destroy_descriptor_pool(Device *d, VkDescriptorPool descriptor_pool) { vkDestroyDescriptorPool(d->device, descriptor_pool, nullptr); }

		DescriptorSet init_descriptor_set(Device *d, VkDescriptorPool pool, Platform::DescriptorLayout *layout)
		{
			DescriptorSet descriptor_set{.layout = layout};
			u32 i = 0;
			for (auto &binding : layout->bindings)
			{
				if (binding.type == Platform::DescriptorSetBindingType::UNIFORM_BUFFER)
				{
					auto padded_size = pad_size(d, binding.size);
					auto buffer      = alloc_buffer(d, padded_size * layout->max_objects, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU,
													VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
					void *mapped     = map_buffer(d, &buffer);
					descriptor_set.binding_buffers.set(i, {.buffer = buffer, .mapped = mapped});
				}
				else if (binding.type == Platform::DescriptorSetBindingType::TEXTURE)
				{
					descriptor_set.binding_samplers.set(i, None);
				}
				i++;
			}

			VkDescriptorSetAllocateInfo info{
				.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.descriptorPool     = pool,
				.descriptorSetCount = 1,
				.pSetLayouts        = &layout->vk_layout,
				.pNext              = nullptr,
			};

			VK_CHECK(vkAllocateDescriptorSets(d->device, &info, &descriptor_set.vk_set));
			return descriptor_set;
		}

		void destroy_descriptor_set(Device *d, DescriptorSet *set_buffer)
		{
			for (auto [binding, binding_buffer] : set_buffer->binding_buffers)
			{
				unmap_buffer(d, &binding_buffer.buffer);
				free_buffer(d, &binding_buffer.buffer);
			}
			set_buffer->binding_buffers.clear();
			set_buffer->binding_samplers.clear();
		}

	} // namespace Vulkan
	namespace Platform
	{
		DescriptorLayout *init_descriptor_layout(RenderContext *c, const Vector<DescriptorSetBinding> &bindings, u32 max_objects)
		{
			ASSERT(max_objects > 0, "Max descriptor objects should be greater than 0!");
			auto *layout        = v_alloc<DescriptorLayout>();
			layout->max_objects = max_objects;
			layout->bindings    = bindings;
			VkDescriptorSetLayoutBinding layout_bindings[bindings.size()];
			u32 i = 0;
			for (auto &binding : bindings)
			{
				VkDescriptorType descriptor_type;
				switch (binding.type)
				{
					case DescriptorSetBindingType::UNIFORM_BUFFER:
						descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
						break;
					case DescriptorSetBindingType::TEXTURE:
						descriptor_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						break;
				}

				layout_bindings[i] = {
					.binding         = i,
					.descriptorCount = 1,
					.descriptorType  = descriptor_type,
					.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
				};
				i++;
			}

			VkDescriptorSetLayoutCreateInfo info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext        = nullptr,
				.flags        = 0,
				.bindingCount = static_cast<u32>(bindings.size()),
				.pBindings    = layout_bindings,
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

		void update_descriptor_set(CmdBuffer *cmd, DescriptorLayout *layout, void *data, u32 index, u32 binding)
		{
			ASSERT(index < layout->max_objects, "Index %u is greater than the max number of objects %u in descriptor layout!", index, layout->max_objects);
			auto *c              = cmd->render_context;
			auto *d              = Vulkan::get_device(c);
			auto *frame          = cmd->frame;

			auto *descriptor_set = &frame->descriptor_sets.get(layout);
			auto *mapped_buffer  = &descriptor_set->binding_buffers.get(binding);

			auto size            = layout->bindings[binding].size;
			auto padded_size     = pad_size(d, size);
			void *start          = reinterpret_cast<byte *>(mapped_buffer->mapped) + index * padded_size;

			if (memcmp(data, start, size) == 0)
				return;

			memcpy(start, data, size);
			descriptor_set->updated = true;
		}

		void update_descriptor_set(CmdBuffer *cmd, DescriptorLayout *layout, Texture *texture, u32 binding)
		{
			auto *frame            = cmd->frame;

			auto *descriptor_set   = &frame->descriptor_sets.get(layout);
			auto *existing_texture = &descriptor_set->binding_samplers.get(binding);

			if (existing_texture->has_value() && texture != nullptr && existing_texture->value() == *texture)
				return;

			*existing_texture       = *texture;
			descriptor_set->updated = true;
		}

		void flush_descriptor_set_changes(CmdBuffer *cmd)
		{
			auto *c     = cmd->render_context;
			auto *frame = cmd->frame;
			Vector<VkWriteDescriptorSet> write_sets{};
			for (auto &[layout, descriptor_set] : frame->descriptor_sets)
			{
				if (!descriptor_set.updated)
					continue;

				descriptor_set.updated = false;
				for (auto [binding, binding_buffer] : descriptor_set.binding_buffers)
				{
					VkDescriptorBufferInfo info{
						.buffer = binding_buffer.buffer.buffer,
						.offset = 0,
						.range  = pad_size(Vulkan::get_device(c), layout->bindings[binding].size) * layout->max_objects,
					};
					write_sets.push_back({
						.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.pNext           = nullptr,
						.dstSet          = descriptor_set.vk_set,
						.dstBinding      = binding,
						.descriptorCount = 1,
						.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
						.pBufferInfo     = &info,
					});
				}

				for (auto [binding, binding_sampler] : descriptor_set.binding_samplers)
				{
					if (!binding_sampler.has_value())
						continue;

					VkDescriptorImageInfo info{
						.sampler     = binding_sampler.value().sampler,
						.imageView   = binding_sampler.value().image_view,
						.imageLayout = binding_sampler.value().layout,
					};
					write_sets.push_back({
						.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.pNext           = nullptr,
						.dstSet          = descriptor_set.vk_set,
						.dstBinding      = binding,
						.descriptorCount = 1,
						.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						.pImageInfo      = &info,
					});
				}
			}
			vkUpdateDescriptorSets(Vulkan::get_device(c)->device, write_sets.size(), write_sets.empty() ? nullptr : &write_sets[0], 0, nullptr);
		}

		void bind_descriptor_set(CmdBuffer *cmd, GraphicsPipeline *pipeline, DescriptorLayout *layout, u32 set, u32 index)
		{
			auto *c              = cmd->render_context;
			auto *frame          = cmd->frame;
			auto *descriptor_set = &frame->descriptor_sets.get(layout);
			Vector<u32> offsets{};
			for (auto &binding : layout->bindings)
			{
				if (binding.type == DescriptorSetBindingType::UNIFORM_BUFFER)
					offsets.push_back(pad_size(Vulkan::get_device(c), binding.size) * index);
			}
			vkCmdBindDescriptorSets(cmd->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_layout, set, 1, &descriptor_set->vk_set, offsets.size(), offsets.empty() ? nullptr : &offsets[0]);
		}
	} // namespace Platform
} // namespace Vultr