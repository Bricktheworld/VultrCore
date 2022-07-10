#include "compute_shader.h"
#include "render_context.h"
#include <shaderc/shaderc.h>

namespace Vultr
{
	namespace Platform
	{
#define ENTRY_NAME "main"
		ErrorOr<Buffer> try_compile_compute_shader(StringView src)
		{
			shaderc_compiler_t compiler         = shaderc_compiler_initialize();
			shaderc_compile_options_t options   = shaderc_compile_options_initialize();

			shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, src.c_str(), src.length(), shaderc_compute_shader, "compute.glsl", ENTRY_NAME, options);
			if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
			{
				auto err = Error("Failed to compile compute shader " + StringView(shaderc_result_get_error_message(result)));
				shaderc_result_release(result);
				shaderc_compile_options_release(options);
				shaderc_compiler_release(compiler);
				return err;
			}

			auto compiled_src = Buffer((const byte *)shaderc_result_get_bytes(result), shaderc_result_get_length(result));

			shaderc_result_release(result);
			shaderc_compile_options_release(options);
			shaderc_compiler_release(compiler);
			return compiled_src;
		}

		ErrorOr<ComputeShader *> try_load_compute_shader(RenderContext *c, const Buffer &compiled_shader)
		{
			auto *d           = Vulkan::get_device(c);
			auto *sc          = Vulkan::get_swapchain(c);
			auto *shader      = v_alloc<ComputeShader>();

			u32 binding_count = 2;
			VkDescriptorSetLayoutBinding layout_bindings[binding_count];
			layout_bindings[0] = {
				.binding         = 0,
				.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = 1,
				.stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT,
			};

			layout_bindings[1] = {
				.binding         = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = 1,
				.stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT,
			};

			VkDescriptorSetLayoutCreateInfo descriptor_layout_info{
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext        = nullptr,
				.flags        = 0,
				.bindingCount = binding_count,
				.pBindings    = layout_bindings,
			};
			VK_TRY(vkCreateDescriptorSetLayout(d->device, &descriptor_layout_info, nullptr, &shader->descriptor_layout));

			for (auto &frame : sc->frames)
				shader->descriptor_pools.push_back(Vulkan::init_descriptor_pool(d));

			for (u32 i = 0; i < Vulkan::MAX_FRAMES_IN_FLIGHT; i++)
			{
				VkDescriptorSetAllocateInfo alloc_info{
					.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
					.descriptorPool     = shader->descriptor_pools[i],
					.descriptorSetCount = 1,
					.pSetLayouts        = &shader->descriptor_layout,
				};

				VkDescriptorSet vk_set;
				VK_TRY(vkAllocateDescriptorSets(d->device, &alloc_info, &vk_set));
				shader->frame_descriptor_sets.push_back(vk_set);
			}

			VkShaderModuleCreateInfo create_info{
				.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = compiled_shader.size(),
				.pCode    = reinterpret_cast<const u32 *>(compiled_shader.storage),
			};
			VK_TRY(vkCreateShaderModule(d->device, &create_info, nullptr, &shader->module));

			VkPipelineShaderStageCreateInfo stage_info{
				.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage  = VK_SHADER_STAGE_COMPUTE_BIT,
				.module = shader->module,
				.pName  = ENTRY_NAME,
			};

			VkPipelineLayoutCreateInfo pipeline_layout_info{
				.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount = 1,
				.pSetLayouts    = &shader->descriptor_layout,
			};
			VK_TRY(vkCreatePipelineLayout(d->device, &pipeline_layout_info, nullptr, &shader->pipeline_layout));

			VkComputePipelineCreateInfo pipeline_info{
				.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
				.pNext  = nullptr,
				.flags  = 0,
				.layout = shader->pipeline_layout,
				.stage  = stage_info,
			};
			VK_TRY(vkCreateComputePipelines(d->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &shader->pipeline));

			return shader;
		}

		void destroy_compute_shader(RenderContext *c, ComputeShader *shader)
		{
			auto *d = Vulkan::get_device(c);

			Vulkan::wait_resource_not_in_use(Vulkan::get_swapchain(c), shader);

			for (auto *pool : shader->descriptor_pools)
				Vulkan::destroy_descriptor_pool(d, pool);

			vkDestroyShaderModule(d->device, shader->module, nullptr);
			vkDestroyPipeline(d->device, shader->pipeline, nullptr);
			vkDestroyPipelineLayout(d->device, shader->pipeline_layout, nullptr);
			vkDestroyDescriptorSetLayout(d->device, shader->descriptor_layout, nullptr);

			v_free(shader);
		}

		void update_compute_shader(ComputeShader *shader, Texture *input, Texture *output)
		{
			ASSERT(input != nullptr && output != nullptr, "Textures provided to compute shader dispatch must not be nullptr!");
			if (input == shader->input && output == shader->output)
				return;

			shader->input  = input;
			shader->output = output;
			shader->updated.set_all();
		}

		void dispatch_compute(CmdBuffer *cmd, ComputeShader *shader)
		{
			auto *c = cmd->render_context;
			auto *d = Vulkan::get_device(c);

			ASSERT(shader->input != nullptr && shader->output != nullptr, "Cannot dispatch compute shader with nullptr images!");

			if (shader->updated.at(cmd->frame_index))
			{
				auto *vk_set                      = shader->frame_descriptor_sets[cmd->frame_index];
				VkDescriptorImageInfo img_infos[] = {
					{
						.sampler     = shader->input->sampler,
						.imageView   = shader->input->image_view,
						.imageLayout = shader->input->layout,
					},
					{
						.sampler     = shader->output->sampler,
						.imageView   = shader->output->image_view,
						.imageLayout = shader->output->layout,
					},
				};
				VkWriteDescriptorSet write_sets[] = {
					VkWriteDescriptorSet{
						.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.dstSet          = vk_set,
						.dstBinding      = 0,
						.descriptorCount = 1,
						.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
						.pImageInfo      = &img_infos[0],
					},
					VkWriteDescriptorSet{
						.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.dstSet          = vk_set,
						.dstBinding      = 1,
						.descriptorCount = 1,
						.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
						.pImageInfo      = &img_infos[1],
					},
				};

				vkUpdateDescriptorSets(d->device, 2, write_sets, 0, nullptr);
				printf("Updated compute descriptor set\n");

				shader->updated.set(cmd->frame_index, false);
			}

			Vulkan::depend_resource(cmd, shader);
			vkCmdBindPipeline(cmd->cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader->pipeline);
			vkCmdBindDescriptorSets(cmd->cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader->pipeline_layout, 0, 1, &shader->frame_descriptor_sets[cmd->frame_index], 0, 0);
			vkCmdDispatch(cmd->cmd_buffer, shader->input->width / 16, shader->input->height / 16, 1);
		}
	} // namespace Platform
} // namespace Vultr
