#include "shader.h"
#include "render_context.h"
#include "constants.h"
#include <vulkan/vulkan.h>
#include <shaderc/shaderc.h>
#include <spirv_reflect/spirv_reflect.h>
#include <vultr_resource_allocator.h>
#include <core/reflection/reflection.h>

namespace Vultr
{
	// Dumb hack so that we can have another consteval for colors specifically for shaders.
	struct Color
	{
	};

	template <>
	inline constexpr Type get_type<Color> = {PrimitiveType::COLOR, []() { return sizeof(Vec4); }, generic_type_serializer<Vec4>, generic_type_deserializer<Vec4>, generic_copy_constructor<Vec4>, "Color"};

	namespace Platform
	{
#define ENTRY_NAME "main"
		static constexpr StringView vertex_pragma   = "#pragma vertex\n";
		static constexpr StringView fragment_pragma = "#pragma fragment\n";

		static u32 get_uniform_type_size(const Type &type)
		{
			switch (type.primitive_type)
			{
				case PrimitiveType::VEC2:
					return sizeof(f32) * 2;
				case PrimitiveType::VEC3:
					return sizeof(f32) * 3;
				case PrimitiveType::VEC4:
				case PrimitiveType::COLOR:
					return sizeof(f32) * 4;
				case PrimitiveType::MAT3:
					return sizeof(f32) * 3 * 3;
				case PrimitiveType::MAT4:
					return sizeof(f32) * 4 * 4;
				case PrimitiveType::F32:
					return sizeof(f32);
				case PrimitiveType::F64:
					return sizeof(f64);
				case PrimitiveType::S8:
					return sizeof(s8);
				case PrimitiveType::S16:
					return sizeof(s16);
				case PrimitiveType::S32:
					return sizeof(s32);
				case PrimitiveType::S64:
					return sizeof(s64);
				case PrimitiveType::U8:
					return sizeof(u8);
				case PrimitiveType::U16:
					return sizeof(u16);
				case PrimitiveType::U32:
					return sizeof(u32);
				case PrimitiveType::U64:
					return sizeof(u64);
				default:
					return 0;
			}
		}

		static u32 get_uniform_type_alignment(const Type &type)
		{
			switch (type.primitive_type)
			{
				case PrimitiveType::VEC2:
					return sizeof(f32) * 2;
				case PrimitiveType::VEC3:
					return sizeof(f32) * 3;
				case PrimitiveType::VEC4:
				case PrimitiveType::COLOR:
					return sizeof(f32) * 4;
				case PrimitiveType::MAT3:
					return sizeof(f32) * 3 * 3;
				case PrimitiveType::MAT4:
					return sizeof(f32) * 4 * 4;
				case PrimitiveType::F32:
					return sizeof(f32);
				case PrimitiveType::F64:
					return sizeof(f64);
				case PrimitiveType::S8:
					return sizeof(s8);
				case PrimitiveType::S16:
					return sizeof(s16);
				case PrimitiveType::S32:
					return sizeof(s32);
				case PrimitiveType::S64:
					return sizeof(s64);
				case PrimitiveType::U8:
					return sizeof(u8);
				case PrimitiveType::U16:
					return sizeof(u16);
				case PrimitiveType::U32:
					return sizeof(u32);
				case PrimitiveType::U64:
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

		static u32 get_uniform_member_offset(const Type &type, u32 next) { return align(next, get_uniform_type_alignment(type)); }

		static ErrorOr<Type> get_uniform_type(const SpvReflectTypeDescription &member)
		{
			switch (member.op)
			{
				case SpvOpTypeVector:
				{
					switch (member.traits.numeric.vector.component_count)
					{
						case 2:
							return get_type<Vec2>;
						case 3:
							return get_type<Vec3>;
						case 4:
							return get_type<Color>;
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
							return get_type<Mat3>;
						}
						case 4:
						{
							return get_type<Mat4>;
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
							return get_type<f32>;
						case 64:
							return get_type<f64>;
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
								return get_type<s8>;
							case 16:
								return get_type<s16>;
							case 32:
								return get_type<s32>;
							case 64:
								return get_type<s64>;
							default:
								return Error("Unsupported int size!");
						}
					}
					else
					{
						switch (member.traits.numeric.scalar.width)
						{
							case 8:
								return get_type<u8>;
							case 16:
								return get_type<u16>;
							case 32:
								return get_type<u32>;
							case 64:
								return get_type<u64>;
							default:
								return Error("Unsupported unsigned int size!");
						}
					}
				}
				default:
					return Error("Unsupported member type!");
			}
		}

		ErrorOr<ShaderReflection> try_reflect_shader(const CompiledShaderSrc &compiled_shader)
		{
			ShaderReflection reflection{};
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
				u32 buffer_offset = 0;
				for (u32 i = 0; i < type_description->member_count; i++)
				{
					auto member            = type_description->members[i];
					StringView member_name = member.struct_member_name;
					TRY_UNWRAP(auto member_type, get_uniform_type(member));
					reflection.uniform_members.push_back({
						.name   = String(member_name),
						.type   = member_type,
						.offset = get_uniform_member_offset(member_type, buffer_offset),
					});
					buffer_offset += get_uniform_type_size(member_type);
				}
				reflection.uniform_size = buffer_offset;
			}

			for (u32 i = 1; i < descriptor_set->binding_count; i++)
			{
				auto *binding = descriptor_set->bindings[i];
				if (binding->descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
					return Error("All bindings after 0 must be samplers!");
				StringView name  = binding->name;
				SamplerType type = SamplerType::ALBEDO;
				if (name == "u_Normal_map")
				{
					type = SamplerType::NORMAL;
				}
				else if (name == "u_Metallic_map")
				{
					type = SamplerType::METALLIC;
				}
				else if (name == "u_Roughness_map")
				{
					type = SamplerType::ROUGHNESS;
				}
				else if (name == "u_Ambient_occlusion_map")
				{
					type = SamplerType::AMBIENT_OCCLUSION;
				}

				reflection.samplers.push_back({
					.name = String(name),
					.type = type,
				});
			}

			spvReflectDestroyShaderModule(&module);
			return reflection;
		}

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

		const ShaderReflection *get_reflection_data(const Shader *shader) { return &shader->reflection; }

		ErrorOr<Shader *> try_load_shader(RenderContext *c, const CompiledShaderSrc &compiled_shader, const ShaderReflection &reflection)
		{
			auto *d      = Vulkan::get_device(c);
			auto *sc     = Vulkan::get_swapchain(c);
			auto *shader = v_alloc<Shader>();
			printf("Allocated shader at %p\n", shader);
			shader->reflection = reflection;

			u32 binding_count = reflection.samplers.size() + 1;
			VkDescriptorSetLayoutBinding layout_bindings[binding_count];
			layout_bindings[0] = {
				.binding         = 0,
				.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			};

			for (u32 i = 1; i < binding_count; i++)
			{
				layout_bindings[i] = {
					.binding         = i,
					.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
				};
			}

			VkDescriptorSetLayoutCreateInfo info = {
				.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext        = nullptr,
				.flags        = 0,
				.bindingCount = binding_count,
				.pBindings    = layout_bindings,
			};
			VK_TRY(vkCreateDescriptorSetLayout(d->device, &info, nullptr, &shader->vk_custom_layout));

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
				set.updated.clear();
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

		template <typename T>
		static ErrorOr<void> read_str(const Vector<StringView> &raw, u32 width, u32 offset, Material *out)
		{
			if (raw.size() != width)
				return Error("Invalid number of values!");

			T parsed[width];
			for (u32 i = 0; i < width; i++)
			{
				if constexpr (is_same<T, f32> || is_same<T, f64>)
				{
					if check (parse_f64(raw[i]), f64 val, auto err)
					{
						parsed[i] = static_cast<T>(val);
					}
					else
					{
						return err;
					}
				}
				else if (is_same<T, u8> || is_same<T, u16> || is_same<T, u32> || is_same<T, u64>)
				{
					if check (parse_u64(raw[i]), u64 val, auto err)
					{
						parsed[i] = static_cast<T>(val);
					}
					else
					{
						return err;
					}
				}
				else if (is_same<T, s8> || is_same<T, s16> || is_same<T, s32> || is_same<T, s64>)
				{
					if check (parse_s64(raw[i]), s64 val, auto err)
					{
						parsed[i] = static_cast<T>(val);
					}
					else
					{
						return err;
					}
				}
			}

			memmove(out->uniform_data + offset, parsed, sizeof(T) * width);

			return Success;
		}

		ErrorOr<Material *> try_load_material(UploadContext *c, const Resource<Shader *> &shader_resource, const StringView &src)
		{
			auto *shader     = shader_resource.value();
			auto *reflection = &shader->reflection;
			auto lines       = split(src, "\n");
			if (lines.size() <= 1)
				return Error("Material does not have enough lines to be valid!");
			if (lines.size() - 1 != reflection->uniform_members.size() + reflection->samplers.size())
				return Error("Material does not match the same number of lines!");

			auto *mat = v_alloc<Material>();
			for (u32 i = 0; i < reflection->uniform_members.size(); i++)
			{
				auto &line       = lines[i + 1];
				auto spl         = split(line, ":");
				auto member_name = spl[0];

				if (member_name != reflection->uniform_members[i].name)
					return Error("Invalid uniform member " + member_name + "found!");

				auto values      = split(spl[1], ",");
				auto member_info = reflection->uniform_members[i];
				auto offset      = member_info.offset;
				switch (member_info.type.primitive_type)
				{
					case PrimitiveType::VEC2:
						TRY(read_str<f32>(values, 2, offset, mat));
						break;
					case PrimitiveType::VEC3:
						TRY(read_str<f32>(values, 3, offset, mat));
						break;
					case PrimitiveType::VEC4:
					case PrimitiveType::COLOR:
						TRY(read_str<f32>(values, 4, offset, mat));
						break;
					case PrimitiveType::MAT3:
						TRY(read_str<f32>(values, 3 * 3, offset, mat));
						break;
					case PrimitiveType::MAT4:
						TRY(read_str<f32>(values, 4 * 4, offset, mat));
						break;
					case PrimitiveType::F32:
						TRY(read_str<f32>(values, 1, offset, mat));
						break;
					case PrimitiveType::F64:
						TRY(read_str<f64>(values, 1, offset, mat));
						break;
					case PrimitiveType::S8:
						TRY(read_str<s8>(values, 1, offset, mat));
						break;
					case PrimitiveType::S16:
						TRY(read_str<s16>(values, 1, offset, mat));
						break;
					case PrimitiveType::S32:
						TRY(read_str<s32>(values, 1, offset, mat));
						break;
					case PrimitiveType::S64:
						TRY(read_str<s64>(values, 1, offset, mat));
						break;
					case PrimitiveType::U8:
						TRY(read_str<u8>(values, 1, offset, mat));
						break;
					case PrimitiveType::U16:
						TRY(read_str<u16>(values, 1, offset, mat));
						break;
					case PrimitiveType::U32:
						TRY(read_str<u32>(values, 1, offset, mat));
						break;
					case PrimitiveType::U64:
						TRY(read_str<u64>(values, 1, offset, mat));
						break;
					default:
						THROW("Invalid member type!");
				}
			}

			for (u32 i = 0; i < reflection->samplers.size(); i++)
			{
				auto &line = lines[i + reflection->uniform_members.size() + 1];
				auto spl   = split(line, ":");

				if (spl[0] != reflection->samplers[i].name)
					return Error("Invalid sampler found!");
				if (spl[1] == VULTR_NULL_FILE_HANDLE)
				{
					mat->samplers.push_back(Resource<Platform::Texture *>());
				}
				else
				{
					mat->samplers.push_back(Resource<Platform::Texture *>(Path(spl[1])));
				}
			}

			mat->source     = shader_resource;
			mat->descriptor = alloc_descriptor_set(c, shader);
			update_descriptor_set(mat->descriptor, mat->uniform_data);

			return mat;
		}

		void bind_material(CmdBuffer *cmd, GraphicsPipeline *pipeline, Material *mat)
		{
			update_descriptor_set(mat->descriptor, mat->uniform_data);
			u32 binding = 1;
			for (auto &sampler : mat->samplers)
			{
				if (!sampler.empty())
				{
					update_descriptor_set(mat->descriptor, ResourceId(sampler), binding);
				}
				else
				{
					update_descriptor_set(mat->descriptor, None, binding);
				}
				binding++;
			}
			bind_pipeline(cmd, pipeline);
			bind_descriptor_set(cmd, pipeline, mat->descriptor);
		}

		void destroy_material(RenderContext *c, Material *mat)
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
				printf("Pushed allocated descriptor set, shader now has %lu\n", shader->allocated_descriptor_sets.size());
			}

			auto padded_size = pad_size(d, shader->reflection.uniform_size);
			auto buffer = alloc_buffer(d, padded_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			void *mapped = map_buffer(d, &buffer);
			descriptor_set->uniform_buffer_binding = {.buffer = buffer, .mapped = mapped};
			descriptor_set->updated.set_all();
			descriptor_set->sampler_bindings.resize(shader->reflection.samplers.size());

			return descriptor_set;
		}

		void free_descriptor_set(RenderContext *c, DescriptorSet *set)
		{
			auto *d      = Vulkan::get_device(c);
			auto *shader = set->shader;
			{
				Platform::Lock lock(shader->mutex);
				auto index = shader->allocated_descriptor_sets.index_of(set);
				shader->allocated_descriptor_sets.remove(index);
				shader->free_descriptor_sets.push(set);
			}
			set->updated.clear();

			unmap_buffer(d, &set->uniform_buffer_binding.buffer);
			free_buffer(Vulkan::get_swapchain(c), &set->uniform_buffer_binding.buffer);

			set->uniform_buffer_binding = {};
			set->sampler_bindings.clear();
		}
	} // namespace Platform
} // namespace Vultr
