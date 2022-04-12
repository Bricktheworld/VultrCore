#include "shader.h"
#include "render_context.h"
#include "constants.h"
#include <vulkan/vulkan.h>
#include <shaderc/shaderc.h>
#include <spirv_reflect/spirv_reflect.h>

namespace Vultr
{
	namespace Platform
	{
#define ENTRY_NAME "main"
		static constexpr StringView vertex_pragma   = "#pragma vertex\n";
		static constexpr StringView fragment_pragma = "#pragma fragment\n";

		ErrorOr<CompiledShaderSrc> try_compile_shader(StringView src)
		{
			if (!contains(src, "#pragma vertex\n"))
				return Error("Source does not contain a vertex shader denoted by '#pragma vertex'");
			if (!contains(src, "#pragma fragment\n"))
				return Error("Source does not contain a fragment shader denoted by '#pragma fragment'");

			shaderc_compiler_t compiler       = shaderc_compiler_initialize();
			shaderc_compile_options_t options = shaderc_compile_options_initialize();

			auto vert_index                   = find(src, vertex_pragma).value() + vertex_pragma.length();
			auto frag_index                   = find(src, fragment_pragma).value() + fragment_pragma.length();
			CompiledShaderSrc compiled_src{};
			{
				auto vert_src                       = String(src.substr(vert_index, frag_index));
				shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, vert_src.c_str(), vert_src.length(), shaderc_vertex_shader, "vertex.glsl", ENTRY_NAME, options);

				if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
				{
					auto err = Error("Failed to compile vertex shader " + StringView(shaderc_result_get_error_message(result)));
					shaderc_result_release(result);
					shaderc_compile_options_release(options);
					shaderc_compiler_release(compiler);
					return err;
				}

				compiled_src.vert_src = Buffer((const byte *)shaderc_result_get_bytes(result), shaderc_result_get_length(result));

				shaderc_result_release(result);
			}

			{
				auto frag_src                       = String(src.substr(frag_index));
				shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, frag_src.c_str(), frag_src.length(), shaderc_fragment_shader, "fragment.glsl", ENTRY_NAME, options);

				if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
				{
					auto err = Error("Failed to compile fragment shader " + StringView(shaderc_result_get_error_message(result)));
					shaderc_result_release(result);
					shaderc_compile_options_release(options);
					shaderc_compiler_release(compiler);
					return err;
				}

				compiled_src.frag_src = Buffer((const byte *)shaderc_result_get_bytes(result), shaderc_result_get_length(result));

				shaderc_result_release(result);
			}

			shaderc_compile_options_release(options);
			shaderc_compiler_release(compiler);
			return compiled_src;
		}

		static u32 get_uniform_type_size(UniformType type)
		{
			switch (type)
			{
				case UniformType::Vec2:
					return sizeof(f32) * 2;
				case UniformType::Vec3:
					return sizeof(f32) * 3;
				case UniformType::Vec4:
					return sizeof(f32) * 4;
				case UniformType::Mat3:
					return sizeof(f32) * 3 * 3;
				case UniformType::Mat4:
					return sizeof(f32) * 4 * 4;
				case UniformType::f32:
					return sizeof(f32);
				case UniformType::f64:
					return sizeof(f64);
				case UniformType::s8:
					return sizeof(s8);
				case UniformType::s16:
					return sizeof(s16);
				case UniformType::s32:
					return sizeof(s32);
				case UniformType::s64:
					return sizeof(s64);
				case UniformType::u8:
					return sizeof(u8);
				case UniformType::u16:
					return sizeof(u16);
				case UniformType::u32:
					return sizeof(u32);
				case UniformType::u64:
					return sizeof(u64);
				default:
					return 0;
			}
		}

		static u32 get_uniform_type_alignment(UniformType type)
		{
			switch (type)
			{
				case UniformType::Vec2:
					return sizeof(f32) * 2;
				case UniformType::Vec3:
					return sizeof(f32) * 3;
				case UniformType::Vec4:
					return sizeof(f32) * 4;
				case UniformType::Mat3:
					return sizeof(f32) * 3 * 3;
				case UniformType::Mat4:
					return sizeof(f32) * 4 * 4;
				case UniformType::f32:
					return sizeof(f32);
				case UniformType::f64:
					return sizeof(f64);
				case UniformType::s8:
					return sizeof(s8);
				case UniformType::s16:
					return sizeof(s16);
				case UniformType::s32:
					return sizeof(s32);
				case UniformType::s64:
					return sizeof(s64);
				case UniformType::u8:
					return sizeof(u8);
				case UniformType::u16:
					return sizeof(u16);
				case UniformType::u32:
					return sizeof(u32);
				case UniformType::u64:
					return sizeof(u64);
				default:
					return 0;
			}
		}

		static size_t align(u32 next, u32 alignment)
		{
			if (next == 0)
				return 0;

			u32 remainder = next % alignment;

			if (remainder == 0)
				return next;

			return next + alignment - remainder;
		}

		static u32 get_uniform_member_offset(UniformType type, u32 next) { return align(next, get_uniform_type_alignment(type)); }

		static ErrorOr<UniformType> get_uniform_type(const SpvReflectTypeDescription &member)
		{
			switch (member.op)
			{
				case SpvOpTypeVector:
				{
					switch (member.traits.numeric.vector.component_count)
					{
						case 2:
							return UniformType::Vec2;
						case 3:
							return UniformType::Vec3;
						case 4:
							return UniformType::Vec4;
						default:
							return Error("Unsupported vector size!");
					}
				}
				case SpvOpTypeMatrix:
				{
					if (member.traits.numeric.matrix.column_count != member.traits.numeric.matrix.row_count)
						return Error("Unsupported matrix with uneven rows and columns!");
					switch (member.traits.numeric.matrix.column_count)
					{
						case 3:
						{
							return UniformType::Mat3;
						}
						case 4:
						{
							return UniformType::Mat4;
						}
						default:
							return Error("Unsupported matrix size!");
					}
				}
				case SpvOpTypeFloat:
				{
					switch (member.traits.numeric.scalar.width)
					{
						case 32:
							return UniformType::f32;
						case 64:
							return UniformType::f64;
						default:
							return Error("Unsupported float size!");
					}
				}
				case SpvOpTypeInt:
				{
					if (member.traits.numeric.scalar.signedness)
					{
						switch (member.traits.numeric.scalar.width)
						{
							case 8:
								return UniformType::s8;
							case 16:
								return UniformType::s16;
							case 32:
								return UniformType::s32;
							case 64:
								return UniformType::s64;
							default:
								return Error("Unsupported int size!");
						}
					}
					else
					{
						switch (member.traits.numeric.scalar.width)
						{
							case 8:
								return UniformType::u8;
							case 16:
								return UniformType::u16;
							case 32:
								return UniformType::u32;
							case 64:
								return UniformType::u64;
							default:
								return Error("Unsupported unsigned int size!");
						}
					}
				}
				default:
					return Error("Unsupported member type!");
			}
		}

		ErrorOr<Shader *> try_load_shader(RenderContext *c, const CompiledShaderSrc &compiled_shader)
		{
			auto *d      = Vulkan::get_device(c);
			auto *sc     = Vulkan::get_swapchain(c);
			auto *shader = v_alloc<Shader>();

			SpvReflectShaderModule module;
			SpvReflectResult result = spvReflectCreateShaderModule(compiled_shader.vert_src.size(), compiled_shader.vert_src.storage, &module);
			if (result != SPV_REFLECT_RESULT_SUCCESS)
				return Error("Something went wrong reflecting shader!");

			u32 descriptor_count = 0;
			result               = spvReflectEnumerateDescriptorSets(&module, &descriptor_count, nullptr);

			if (result != SPV_REFLECT_RESULT_SUCCESS)
				return Error("Something went wrong getting the number of descriptor sets for shader!");

			if (descriptor_count < 1)
				return Error("Shader must contain at least one descriptor!");

			SpvReflectDescriptorSet *descriptor_set;

			{
				SpvReflectDescriptorSet *descriptor_sets[descriptor_count];
				result = spvReflectEnumerateDescriptorSets(&module, &descriptor_count, descriptor_sets);

				if (result != SPV_REFLECT_RESULT_SUCCESS)
					return Error("Something went wrong enumerating descriptor sets!");

				if (descriptor_count != 2)
					return Error("Only 2 descriptors are supported in shaders (default and one additional for material data)!");

				descriptor_set = descriptor_sets[1];
				ASSERT(descriptor_set->binding_count >= 1, "Descriptor must have at least one binding!");
			}

			{
				auto *binding = descriptor_set->bindings[0];
				auto type     = binding->descriptor_type;

				if (type != SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
					return Error("Uniform buffer must be the first binding in the descriptor set!");

				auto *type_description = binding->type_description;

				if (type_description->op != SpvOpTypeStruct)
					return Error("Uniform buffer must be a struct!");

				ASSERT(type_description->member_count >= 1, "Uniform buffer struct must have at least one member!");
				shader->uniform_members.resize(type_description->member_count);
				u32 buffer_offset = 0;
				for (u32 i = 0; i < type_description->member_count; i++)
				{
					auto member            = type_description->members[i];
					StringView member_name = member.struct_member_name;
					TRY_UNWRAP(UniformType member_type, get_uniform_type(member));
					shader->uniform_member_names.set(member_name, i);
					shader->uniform_members[i] = {
						.type    = member_type,
						.binding = i,
						.offset  = get_uniform_member_offset(member_type, buffer_offset),
					};
					buffer_offset += get_uniform_type_size(member_type);
				}
				shader->uniform_size = buffer_offset;
			}

			VkDescriptorSetLayoutBinding layout_bindings[descriptor_set->binding_count];
			layout_bindings[0] = {
				.binding         = 0,
				.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			};

			for (u32 i = 1; i < descriptor_set->binding_count; i++)
			{
				auto *binding = descriptor_set->bindings[i];
				if (binding->descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
					return Error("All bindings after 0 must be samplers!");

				layout_bindings[i] = {
					.binding         = i,
					.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
				};

				shader->sampler_bindings.set(binding->name, i);
			}

			VkDescriptorSetLayoutCreateInfo info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext        = nullptr,
				.flags        = 0,
				.bindingCount = static_cast<u32>(descriptor_set->binding_count),
				.pBindings    = layout_bindings,
			};
			VK_TRY(vkCreateDescriptorSetLayout(d->device, &info, nullptr, &shader->vk_custom_layout));

			spvReflectDestroyShaderModule(&module);

			VkShaderModuleCreateInfo create_info{
				.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = compiled_shader.vert_src.size(),
				.pCode    = reinterpret_cast<const u32 *>(compiled_shader.vert_src.storage),
			};

			VK_TRY(vkCreateShaderModule(d->device, &create_info, nullptr, &shader->vert_module))

			create_info = {
				.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = compiled_shader.frag_src.size(),
				.pCode    = reinterpret_cast<const u32 *>(compiled_shader.frag_src.storage),
			};

			VK_TRY(vkCreateShaderModule(d->device, &create_info, nullptr, &shader->frag_module))

			for (auto &frame : sc->frames)
				shader->vk_descriptor_pools.push_back(Vulkan::init_descriptor_pool(d));

			for (auto &set : shader->descriptor_set_pool)
			{
				set.shader                 = shader;
				set.uniform_buffer_binding = {};
				set.sampler_bindings       = {};
				for (u32 i = 0; i < sc->frames.size(); i++)
				{
					VkDescriptorSetAllocateInfo alloc_info{
						.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
						.pNext              = nullptr,
						.descriptorPool     = shader->vk_descriptor_pools[i],
						.descriptorSetCount = 1,
						.pSetLayouts        = &shader->vk_custom_layout,
					};

					VkDescriptorSet vk_set;
					VK_CHECK(vkAllocateDescriptorSets(d->device, &alloc_info, &vk_set));
					set.vk_frame_descriptor_sets.push_back(vk_set);
				}
				set.updated = 0;
				shader->free_descriptor_sets.push(&set);
			}

			return shader;
		}

		void destroy_shader(RenderContext *c, Shader *shader)
		{
			auto *d = Vulkan::get_device(c);

			ASSERT(shader->vert_module != nullptr && shader->frag_module != nullptr, "Cannot destroy shader with module nullptr");
			ASSERT(shader->allocated_descriptor_sets.empty(), "Destroying shader before depending descriptor sets have been freed!");
			vkDestroyShaderModule(d->device, shader->vert_module, nullptr);
			vkDestroyShaderModule(d->device, shader->frag_module, nullptr);

			vkDestroyDescriptorSetLayout(d->device, shader->vk_custom_layout, nullptr);
			for (auto &pool : shader->vk_descriptor_pools)
			{
				Vulkan::destroy_descriptor_pool(d, pool);
			}

			shader->vert_module      = nullptr;
			shader->frag_module      = nullptr;
			shader->vk_custom_layout = nullptr;

			v_free(shader);
		}

		static ErrorOr<void> read_floats(const Vector<StringView> &raw, u32 width, u32 offset, Material *out)
		{
			if (raw.size() != width)
				return Error("Invalid number of values!");

			f32 parsed[width];
			for (u32 i = 0; i < width; i++)
				parsed[i] = static_cast<f32>(parse_f64(raw[i]));

			memmove(out->uniform_data + offset, parsed, sizeof(f32) * width);

			return Success;
		}

		ErrorOr<Material *> try_load_material(UploadContext *c, const Resource<Shader *> &shader_resource, const StringView &src)
		{
			auto *shader = shader_resource.value();
			auto lines   = split(src, "\n");
			if (lines.size() <= 1)
				return Error("Material does not have enough lines to be valid!");
			if (lines.size() - 1 != shader->uniform_members.size() + shader->sampler_bindings.size())
				return Error("Material does not match the same number of lines!");

			auto *mat = v_alloc<Material>();
			for (u32 i = 1; i < shader->uniform_members.size(); i++)
			{
				auto &line       = lines[i];
				auto spl         = split(line, ":");
				auto member_name = spl[0];
				auto values      = split(spl[1], ",");
				auto member_info = shader->uniform_members[shader->uniform_member_names.get(member_name)];
				auto offset      = member_info.offset;
				switch (member_info.type)
				{
					case UniformType::Vec2:
						TRY(read_floats(values, 2, offset, mat));
						break;
					case UniformType::Vec3:
						TRY(read_floats(values, 3, offset, mat));
						break;
					case UniformType::Vec4:
						TRY(read_floats(values, 4, offset, mat));
						break;
					case UniformType::Mat3:
						TRY(read_floats(values, 3 * 3, offset, mat));
						break;
					case UniformType::Mat4:
						TRY(read_floats(values, 4 * 4, offset, mat));
						break;
					case UniformType::f32:
					case UniformType::f64:
						TRY(read_floats(values, 1, offset, mat));
						break;
					case UniformType::s8:
					case UniformType::s16:
					case UniformType::s32:
					case UniformType::s64:
						THROW("Not implemented!");
					case UniformType::u8:
					case UniformType::u16:
					case UniformType::u32:
					case UniformType::u64:
						THROW("Not implemented!");
				}
			}

			mat->samplers.resize(shader->sampler_bindings.size());
			for (u32 i = 0; i < shader->sampler_bindings.size(); i++)
			{
				auto &line                 = lines[i + shader->uniform_members.size() + 1];
				auto spl                   = split(line, ":");
				u32 binding                = shader->sampler_bindings.get(spl[0]);
				mat->samplers[binding - 1] = Resource<Platform::Texture *>(Path(spl[1]));
			}

			mat->source     = shader_resource;
			mat->descriptor = alloc_descriptor_set(c, shader);
			update_descriptor_set(mat->descriptor, mat->uniform_data);

			return mat;
		}

		Hashmap<String, u32> *get_uniform_bindings(Shader *shader) { return &shader->uniform_member_names; }
		Vector<UniformMember> *get_uniform_members(Shader *shader) { return &shader->uniform_members; }
		Hashmap<String, u32> *get_sampler_bindings(Shader *shader) { return &shader->sampler_bindings; }

		void bind_material(CmdBuffer *cmd, GraphicsPipeline *pipeline, Material *mat)
		{
			update_descriptor_set(mat->descriptor, mat->uniform_data);
			u32 binding = 1;
			for (auto &sampler : mat->samplers)
			{
				update_descriptor_set(mat->descriptor, sampler, binding);
				binding++;
			}
			bind_pipeline(cmd, pipeline);
			bind_descriptor_set(cmd, pipeline, mat->descriptor);
		}

		void destroy_material(UploadContext *c, Material *mat)
		{
			free_descriptor_set(c, mat->descriptor);
			mat->samplers.clear();
			mat->source = {};
			v_free(mat);
		}

		DescriptorSet *alloc_descriptor_set(UploadContext *c, Shader *shader)
		{
			DescriptorSet *descriptor_set;
			auto *d = Vulkan::get_device(c);
			{
				Platform::Lock lock(shader->mutex);
				ASSERT(!shader->free_descriptor_sets.empty(), "Maximum number of descriptor sets have been allocated!");
				descriptor_set = shader->free_descriptor_sets.pop();
				shader->allocated_descriptor_sets.push_back(descriptor_set);
			}

			auto padded_size = pad_size(d, shader->uniform_size);
			auto buffer      = alloc_buffer(d, padded_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			void *mapped     = map_buffer(d, &buffer);
			descriptor_set->uniform_buffer_binding = {.buffer = buffer, .mapped = mapped};
			descriptor_set->updated                = 0;
			descriptor_set->sampler_bindings.resize(shader->sampler_bindings.size());

			return descriptor_set;
		}

		void free_descriptor_set(UploadContext *c, DescriptorSet *set)
		{
			auto *d      = Vulkan::get_device(c);
			auto *shader = set->shader;
			{
				Platform::Lock lock(shader->mutex);
				auto index = shader->allocated_descriptor_sets.index_of(set);
				shader->allocated_descriptor_sets.remove(index);
				shader->free_descriptor_sets.push(set);
			}
			set->updated = 0;

			unmap_buffer(d, &set->uniform_buffer_binding.buffer);
			free_buffer(d, &set->uniform_buffer_binding.buffer);

			set->uniform_buffer_binding = {};
			set->sampler_bindings.clear();
		}
	} // namespace Platform
} // namespace Vultr
