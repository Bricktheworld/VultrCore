#pragma once
#include "platform.h"
#include <filesystem/filestream.h>
#include <types/vector.h>
#include <types/buffer.h>
#include <glm/glm.hpp>
#include <filesystem/filesystem.h>
#include <imgui/imgui.h>
#include <core/resource_allocator/resource_allocator.h>
#include <core/reflection/reflection.h>

namespace Vultr
{
	namespace Platform
	{
		struct RenderContext;
		RenderContext *init_render_context(const Window *window, bool debug);
		RenderContext *get_render_context(const Window *window);
		void framebuffer_resize_callback(const Window *window, RenderContext *c, u32 width, u32 height);
		void destroy_render_context(RenderContext *c);

		struct UploadContext;
		UploadContext *init_upload_context(RenderContext *c);
		void destroy_upload_context(UploadContext *upload_context);

		struct Vertex
		{
			Vec3 position;
			Vec3 normal;
			Vec2 uv;
			Vec3 tangent;
			Vec3 bitangent;
		};

		enum struct VertexAttributeType
		{
			F32,
			F32_VEC2,
			F32_VEC3,
			F32_VEC4,
			S32,
			S32_VEC2,
			S32_VEC3,
			S32_VEC4,
			U32,
			U32_VEC2,
			U32_VEC3,
			U32_VEC4,
			F64,
			F64_VEC2,
			F64_VEC3,
			F64_VEC4,
		};

		struct VertexAttributeDescription
		{
			u32 offset;
			VertexAttributeType type;
		};

		struct VertexDescription
		{
			u32 stride;
			Vector<VertexAttributeDescription> attribute_descriptions;
		};

		template <typename T>
		VertexDescription get_vertex_description();

		template <>
		inline VertexDescription get_vertex_description<Vertex>()
		{
			return {
				.stride                 = sizeof(Vertex),
				.attribute_descriptions = Vector<VertexAttributeDescription>({
					{
						.offset = offsetof(Vertex, position),
						.type   = VertexAttributeType::F32_VEC3,
					},
					{
						.offset = offsetof(Vertex, normal),
						.type   = VertexAttributeType::F32_VEC3,
					},
					{
						.offset = offsetof(Vertex, uv),
						.type   = VertexAttributeType::F32_VEC2,
					},
					{
						.offset = offsetof(Vertex, tangent),
						.type   = VertexAttributeType::F32_VEC3,
					},
					{
						.offset = offsetof(Vertex, bitangent),
						.type   = VertexAttributeType::F32_VEC3,
					},
				}),
			};
		}

		struct Texture;

		enum struct TextureFormat
		{
			RGB8,
			RGB16,
			RGBA8,
			RGBA16,
			SRGB8,
			SRGBA8,
			RGB8_CUBEMAP,
			RGB16_CUBEMAP,
			RGBA8_CUBEMAP,
			RGBA16_CUBEMAP,
			SRGB8_CUBEMAP,
			SRGBA8_CUBEMAP,
			DEPTH,
		};

		inline TextureFormat texture_format_from_string(StringView string)
		{
			if (string == "RGB8")
				return TextureFormat::RGB8;
			else if (string == "RGB16")
				return TextureFormat::RGB16;
			else if (string == "RGBA8")
				return TextureFormat::RGBA8;
			else if (string == "RGBA16")
				return TextureFormat::RGBA16;
			else if (string == "SRGB8")
				return TextureFormat::SRGB8;
			else if (string == "SRGBA8")
				return TextureFormat::SRGBA8;
			else if (string == "RGB8_CUBEMAP")
				return TextureFormat::RGB8_CUBEMAP;
			else if (string == "RGB16_CUBEMAP")
				return TextureFormat::RGB16_CUBEMAP;
			else if (string == "RGBA8_CUBEMAP")
				return TextureFormat::RGBA8_CUBEMAP;
			else if (string == "RGBA16_CUBEMAP")
				return TextureFormat::RGBA16_CUBEMAP;
			else if (string == "SRGB8_CUBEMAP")
				return TextureFormat::SRGB8_CUBEMAP;
			else if (string == "SRGBA8_CUBEMAP")
				return TextureFormat::SRGBA8_CUBEMAP;
			else if (string == "DEPTH")
				return TextureFormat::DEPTH;
			else
				return TextureFormat::RGBA8;
		}

		inline constexpr const char *texture_format_to_string(TextureFormat format)
		{
			switch (format)
			{
				case TextureFormat::RGB8:
					return "RGB8";
				case TextureFormat::RGB16:
					return "RGB16";
				case TextureFormat::RGBA8:
					return "RGBA8";
				case TextureFormat::RGBA16:
					return "RGBA16";
				case TextureFormat::SRGB8:
					return "SRGB8";
				case TextureFormat::SRGBA8:
					return "SRGBA8";
				case TextureFormat::RGB8_CUBEMAP:
					return "RGB8_CUBEMAP";
				case TextureFormat::RGB16_CUBEMAP:
					return "RGB16_CUBEMAP";
				case TextureFormat::RGBA8_CUBEMAP:
					return "RGBA8_CUBEMAP";
				case TextureFormat::RGBA16_CUBEMAP:
					return "RGBA16_CUBEMAP";
				case TextureFormat::SRGB8_CUBEMAP:
					return "SRGB8_CUBEMAP";
				case TextureFormat::SRGBA8_CUBEMAP:
					return "SRGBA8_CUBEMAP";
				case TextureFormat::DEPTH:
					return "DEPTH";
			}
		}

		namespace TextureUsage
		{
			constexpr u16 ATTACHMENT = 0x1;
			constexpr u16 TEXTURE    = 0x2;
			constexpr u16 STORAGE    = 0x4;
		} // namespace TextureUsage

		inline u32 get_pixel_size(TextureFormat format)
		{
			switch (format)
			{
				case TextureFormat::RGB8:
				case TextureFormat::RGB8_CUBEMAP:
					return 3 * 2;
				case TextureFormat::RGB16:
				case TextureFormat::RGB16_CUBEMAP:
					return 3;
				case TextureFormat::RGBA8:
				case TextureFormat::RGBA8_CUBEMAP:
					return 4;
				case TextureFormat::RGBA16:
				case TextureFormat::RGBA16_CUBEMAP:
					return 4 * 2;
				case TextureFormat::SRGB8:
				case TextureFormat::SRGB8_CUBEMAP:
					return 3;
				case TextureFormat::SRGBA8:
				case TextureFormat::SRGBA8_CUBEMAP:
					return 4;
				case TextureFormat::DEPTH:
					return 1;
			}
		}

		Texture *init_texture(RenderContext *c, u32 width, u32 height, TextureFormat format, u16 texture_usage);
		Texture *init_texture(UploadContext *c, u32 width, u32 height, TextureFormat format, u16 texture_usage);

		void fill_texture(UploadContext *c, Texture *texture, byte *data, u32 size);

		struct ImportedTexture
		{
			u32 width;
			u32 height;
			Buffer src;
		};

		ErrorOr<void> import_texture_file(const Path &path, ImportedTexture *out, TextureFormat format = TextureFormat::SRGBA8, bool flip_on_load = true);
		ErrorOr<void> import_texture_memory(byte *data, u64 size, ImportedTexture *out, TextureFormat format = TextureFormat::SRGBA8, bool flip_on_load = true);
		Texture *init_white_texture(UploadContext *c);
		Texture *init_black_texture(UploadContext *c);
		Texture *init_normal_texture(UploadContext *c);

		u32 get_width(Texture *texture);
		u32 get_height(Texture *texture);
		void destroy_texture(RenderContext *c, Texture *texture);

		struct Subpass;
		struct RenderPass;

		struct Framebuffer;

		enum struct LoadOp
		{
			DONT_CARE,
			CLEAR,
			LOAD,
		};

		enum struct StoreOp
		{
			DONT_CARE,
			STORE,
		};

		struct AttachmentDescription
		{
			TextureFormat format = TextureFormat::RGBA8;
			u16 texture_usage    = TextureUsage::ATTACHMENT;

			LoadOp load_op       = LoadOp::CLEAR;
			StoreOp store_op     = StoreOp::STORE;
		};

		Framebuffer *init_framebuffer(RenderContext *c, const Vector<AttachmentDescription> &attachments, Option<u32> width = None, Option<u32> height = None);
		Texture *get_attachment_texture(Framebuffer *framebuffer, u32 index);
		u32 get_width(Framebuffer *framebuffer);
		u32 get_height(Framebuffer *framebuffer);
		void destroy_framebuffer(RenderContext *c, Framebuffer *framebuffer);

		struct Shader;

		struct UniformMember
		{
			String name{};
			Type type  = get_type<Vec3>;
			u32 offset = 0;
		};

		struct Uniform
		{
			Vector<UniformMember> members{};
			u32 size = 0;
		};

		enum struct SamplerType
		{
			ALBEDO,
			NORMAL,
			METALLIC,
			ROUGHNESS,
			AMBIENT_OCCLUSION,
			CUBEMAP,
		};

		struct SamplerBinding
		{
			String name{};
			SamplerType type = SamplerType::ALBEDO;
		};

		struct ShaderReflection
		{
			// Non-compute shaders will only support one uniform buffer (that is required) for simplicity.
			Uniform uniform;
			Vector<SamplerBinding> samplers{};
		};

		struct CompiledShaderSrc
		{
			Buffer vert_src{};
			Buffer frag_src{};
		};

		struct DescriptorSet;

		DescriptorSet *alloc_descriptor_set(UploadContext *c, Shader *shader);
		void free_descriptor_set(RenderContext *c, DescriptorSet *set, Shader *shader);

		static constexpr u64 MAX_MATERIAL_SIZE = Kilobyte(2);

		struct Material
		{
			byte uniform_data[MAX_MATERIAL_SIZE]{};
			Vector<Resource<Platform::Texture *>> samplers{};
			Resource<Platform::Shader *> source{};
			DescriptorSet *descriptor = nullptr;
		};

		ErrorOr<CompiledShaderSrc> try_compile_shader(StringView src);
		ErrorOr<ShaderReflection> try_reflect_shader(const CompiledShaderSrc &compiled_shader);
		ErrorOr<Shader *> try_load_shader(RenderContext *c, const CompiledShaderSrc &compiled_shader, const ShaderReflection &reflection);
		const ShaderReflection *get_reflection_data(const Shader *shader);
		void destroy_shader(RenderContext *c, Shader *shader);

		struct ComputeShader;

		enum struct DescriptorType
		{
			UNIFORM_BUFFER,
			SAMPLER,
			STORAGE_TEXTURE,
		};

		struct ComputeShaderReflection
		{
			Vector<DescriptorType> bindings{};
		};

		ErrorOr<Buffer> try_compile_compute_shader(StringView src);
		ErrorOr<ComputeShaderReflection> try_reflect_compute_shader(const Buffer &compiled_shader);
		ErrorOr<ComputeShader *> try_load_compute_shader(RenderContext *c, const Buffer &compiled_shader, const ComputeShaderReflection &reflection);
		void destroy_compute_shader(RenderContext *c, ComputeShader *shader);

		static constexpr StringView VULTR_NULL_FILE_HANDLE = "VULTR_NULL_FILE";
		ErrorOr<Material *> try_load_material(UploadContext *c, const Resource<Shader *> &shader, const StringView &src);
		void destroy_material(RenderContext *c, Material *mat);

		struct VertexBuffer;
		struct IndexBuffer;

		template <typename T>
		struct MeshT
		{
			T *vertices                 = nullptr;
			u32 vertex_count            = 0;
			u16 *indices                = nullptr;
			u32 index_count             = 0;
			VertexBuffer *vertex_buffer = nullptr;
			IndexBuffer *index_buffer   = nullptr;
		};

		typedef MeshT<Vertex> Mesh;

		VertexBuffer *init_vertex_buffer(UploadContext *c, void *data, size_t size);
		void destroy_vertex_buffer(RenderContext *c, VertexBuffer *buffer);

		IndexBuffer *init_index_buffer(UploadContext *c, u16 *data, size_t size);
		void destroy_index_buffer(RenderContext *c, IndexBuffer *buffer);

		inline Mesh *init_mesh(UploadContext *c, Vertex *vertices, size_t vertex_count, u16 *indices, size_t index_count)
		{
			auto *mesh         = v_alloc<Mesh>();

			mesh->vertices     = v_alloc<Vertex>(vertex_count);
			mesh->vertex_count = vertex_count;
			Utils::copy(mesh->vertices, vertices, vertex_count);

			// Flipping the UVs is necessary so that textures are not inverted.
			for (size_t i = 0; i < vertex_count; i++)
				mesh->vertices[i].uv.y = 1 - mesh->vertices[i].uv.y;

			mesh->vertex_buffer = init_vertex_buffer(c, mesh->vertices, sizeof(Vertex) * vertex_count);

			mesh->indices       = v_alloc<u16>(index_count);
			mesh->index_count   = index_count;
			Utils::copy(mesh->indices, indices, index_count);

			mesh->index_buffer = init_index_buffer(c, mesh->indices, sizeof(u16) * index_count);
			return mesh;
		}

		inline Mesh *init_fullscreen_quad(UploadContext *c)
		{
			Vertex vertices[] = {
				{.position = Vec3(-1, -1, 0), .uv = Vec2(0, 1)},
				{.position = Vec3(-1, 1, 0), .uv = Vec2(0, 0)},
				{.position = Vec3(1, -1, 0), .uv = Vec2(1, 1)},
				{.position = Vec3(1, 1, 0), .uv = Vec2(1, 0)},
			};
			u16 indices[] = {
				1, 2, 3, 1, 0, 2,
			};
			return init_mesh(c, vertices, 4, indices, 6);
		}

		inline Mesh *init_skybox(UploadContext *c)
		{
			Vertex vertices[] = {
				{.position = Vec3(-1, -1, -1)}, // 0
				{.position = Vec3(1, -1, -1)},  // 1
				{.position = Vec3(1, 1, -1)},   // 2
				{.position = Vec3(1, 1, 1)},    // 3
				{.position = Vec3(-1, 1, -1)},  // 4
				{.position = Vec3(-1, 1, 1)},   // 5
				{.position = Vec3(-1, -1, 1)},  // 6
				{.position = Vec3(1, -1, 1)},   // 7
				{.position = Vec3(1, 1, -1)},   // 8
			};

			u16 indices[] = {1, 8, 4, 4, 0, 1,
							 // Left
							 6, 0, 4, 4, 5, 6,
							 // Right
							 1, 7, 3, 3, 8, 1,
							 // Back
							 6, 5, 3, 3, 7, 6,
							 // Top
							 4, 8, 3, 3, 5, 4,
							 // Bottom
							 0, 6, 1, 1, 6, 7};
			return init_mesh(c, vertices, 9, indices, 36);
		}

		inline Mesh *init_sphere(UploadContext *c, u32 radius, u32 latitudes, u32 longitudes)
		{
			Vector<Vertex> vertices{};
			Vector<u16> indices{};

			f32 length_inv          = 1.0f / radius;

			static constexpr f32 PI = 3.14159265359;

			f32 delta_latitude      = PI / latitudes;
			f32 delta_longitude     = 2 * PI / longitudes;
			f32 latitude_angle;
			f32 longitude_angle;

			for (u32 i = 0; i <= latitudes; i++)
			{
				latitude_angle = PI / 2 - i * delta_latitude;
				f32 xy         = radius * cosf(latitude_angle);
				f32 z          = radius * sinf(latitude_angle);

				for (u32 j = 0; j <= longitudes; j++)
				{
					longitude_angle = j * delta_longitude;

					Vertex vertex;
					vertex.position.x = xy * cosf(longitude_angle);
					vertex.position.y = xy * sinf(longitude_angle);
					vertex.position.z = z;
					vertex.uv.x       = (f32)j / longitudes;
					vertex.uv.y       = (f32)i / latitudes;
					vertex.normal.x   = vertex.position.x * length_inv;
					vertex.normal.y   = vertex.position.y * length_inv;
					vertex.normal.z   = vertex.position.z * length_inv;
					vertices.push_back(vertex);
				}
			}

			for (u32 i = 0; i < latitudes; i++)
			{
				u32 k1 = i * (longitudes + 1);
				u32 k2 = k1 + longitudes + 1;

				for (u32 j = 0; j < longitudes; j++, k1++, k2++)
				{
					if (i != 0)
					{
						indices.push_back(k1);
						indices.push_back(k2);
						indices.push_back(k1 + 1);
					}

					if (i != (latitudes - 1))
					{
						indices.push_back(k1 + 1);
						indices.push_back(k2);
						indices.push_back(k2 + 1);
					}
				}
			}

			return init_mesh(c, &vertices[0], vertices.size(), &indices[0], indices.size());
		}

		struct ImportedMesh
		{
			Vertex *vertices = nullptr;
			u32 vertex_count = 0;
			u16 *indices     = nullptr;
			u32 index_count  = 0;
		};

		ErrorOr<ImportedMesh> import_mesh_file(const Path &path);
		ErrorOr<ImportedMesh> import_mesh_memory(const Buffer &buffer);
		void free_imported_mesh(ImportedMesh *mesh);

		inline Mesh *load_mesh_memory(UploadContext *c, const Buffer &vertex_buffer, const Buffer &index_buffer)
		{
			auto *mesh         = v_alloc<Mesh>();

			mesh->vertex_count = vertex_buffer.size() / sizeof(Vertex);
			mesh->vertices     = v_alloc<Vertex>(mesh->vertex_count);

			// Flipping the UVs is necessary so that textures are not inverted.
			for (size_t i = 0; i < mesh->vertex_count; i++)
				mesh->vertices[i].uv.y = 1 - mesh->vertices[i].uv.y;

			Utils::move((byte *)mesh->vertices, vertex_buffer.storage, vertex_buffer.size());

			mesh->vertex_buffer = init_vertex_buffer(c, mesh->vertices, sizeof(Vertex) * mesh->vertex_count);

			mesh->index_count   = index_buffer.size() / sizeof(u16);
			mesh->indices       = v_alloc<u16>(mesh->index_count);

			Utils::move((byte *)mesh->indices, index_buffer.storage, index_buffer.size());

			mesh->index_buffer = init_index_buffer(c, mesh->indices, sizeof(u16) * mesh->index_count);

			return mesh;
		}

		inline void destroy_mesh(RenderContext *c, Mesh *mesh)
		{
			destroy_index_buffer(c, mesh->index_buffer);
			destroy_vertex_buffer(c, mesh->vertex_buffer);
			v_free(mesh->vertices);
			v_free(mesh->indices);
			v_free(mesh);
		}

		struct CameraUBO
		{
			Vec4 position{};
			Mat4 view{};
			Mat4 proj{};
			Mat4 view_proj{};
		};

		struct DirectionalLightUBO
		{
			Vec4 direction{};
			Vec4 diffuse{};
			f32 specular;
			f32 intensity;
			f32 ambient;
		};

		struct PushConstant
		{
			Mat4 model{};
		};

		enum struct DepthTest
		{
			ALWAYS,
			NEVER,
			LESS,
			EQUAL,
			LEQUAL,
			GREATER,
			NOTEQUAL,
			GEQUAL,
		};

		enum struct ShaderUsage
		{
			VERT_FRAG,
			VERT_ONLY,
			FRAG_ONLY,
		};

		struct GraphicsPipelineInfo
		{
			Option<DepthTest> depth_test = DepthTest::LESS;
			bool write_depth             = true;
			ShaderUsage shader_usage     = ShaderUsage::VERT_FRAG;

			bool operator==(const GraphicsPipelineInfo &other) const { return depth_test == other.depth_test && write_depth == other.write_depth; }
		};

		struct CmdBuffer;

		ErrorOr<CmdBuffer *> begin_cmd_buffer(const Window *window);

		void begin_window_framebuffer(CmdBuffer *cmd, Vec4 clear_color = Vec4(0, 0, 0, 1));
		void begin_framebuffer(CmdBuffer *cmd, Framebuffer *framebuffer, Vec4 clear_color = Vec4(0, 0, 0, 1));
		void end_framebuffer(CmdBuffer *cmd);

		void bind_vertex_buffer(CmdBuffer *cmd, VertexBuffer *vbo);
		void bind_index_buffer(CmdBuffer *cmd, IndexBuffer *ibo);
		void draw_indexed(CmdBuffer *cmd, u32 index_count, u32 instance_count = 1, u32 first_index = 0, s32 vertex_offset = 0, u32 first_instance_id = 0);
		void update_descriptor_set(DescriptorSet *set, void *data, size_t size);
		void update_descriptor_set(DescriptorSet *set, Texture *texture, u32 binding);
		void update_default_descriptor_set(CmdBuffer *cmd, CameraUBO *camera_ubo, DirectionalLightUBO *directional_light_ubo);
		void bind_descriptor_set(CmdBuffer *cmd, Shader *shader, DescriptorSet *set, const GraphicsPipelineInfo &info = {}, const Option<PushConstant> &constant = None);
		void bind_material(CmdBuffer *cmd, Material *mat, const GraphicsPipelineInfo &info = {}, const Option<PushConstant> &constant = None);
		void bind_depth_only(CmdBuffer *cmd);
		void update_compute_shader(ComputeShader *shader, Texture *input, Texture *output);
		void dispatch_compute(CmdBuffer *cmd, ComputeShader *shader);

		inline void bind_mesh(CmdBuffer *cmd, Mesh *mesh)
		{
			ASSERT(mesh != nullptr, "Cannot draw nullptr mesh!");
			bind_vertex_buffer(cmd, mesh->vertex_buffer);
			bind_index_buffer(cmd, mesh->index_buffer);
		}
		inline void draw_mesh(CmdBuffer *cmd, Mesh *mesh)
		{
			bind_mesh(cmd, mesh);
			draw_indexed(cmd, mesh->index_count);
		}
		void end_cmd_buffer(CmdBuffer *cmd);

		void wait_idle(RenderContext *c);

		struct ImGuiContext;
		ImGuiContext *init_imgui(const Window *window, UploadContext *upload_context, byte *font_src, size_t font_src_size, u32 font_size);
		void imgui_begin_frame(CmdBuffer *cmd, ImGuiContext *c);
		void imgui_end_frame(CmdBuffer *cmd, ImGuiContext *c);
		ImTextureID imgui_get_texture_id(Texture *texture);
		void imgui_free_texture_id(Texture *texture);
		void destroy_imgui(RenderContext *c, ImGuiContext *imgui_c);

	} // namespace Platform

	template <>
	inline constexpr Type get_type<Resource<Platform::Material *>> = {PrimitiveType::MATERIAL_RESOURCE,
																	  []() { return sizeof(Resource<Platform::Material *>); },
																	  generic_type_serializer<Resource<Platform::Material *>>,
																	  generic_type_deserializer<Resource<Platform::Material *>>,
																	  generic_copy_constructor<Resource<Platform::Material *>>,
																	  "Material"};

	template <>
	inline constexpr Type get_type<Resource<Platform::Texture *>> = {PrimitiveType::TEXTURE_RESOURCE,
																	 []() { return sizeof(Resource<Platform::Texture *>); },
																	 generic_type_serializer<Resource<Platform::Texture *>>,
																	 generic_type_deserializer<Resource<Platform::Texture *>>,
																	 generic_copy_constructor<Resource<Platform::Texture *>>,
																	 "Texture"};

	template <>
	inline constexpr Type get_type<Resource<Platform::Shader *>> = {PrimitiveType::SHADER_RESOURCE,
																	[]() { return sizeof(Resource<Platform::Shader *>); },
																	generic_type_serializer<Resource<Platform::Shader *>>,
																	generic_type_deserializer<Resource<Platform::Shader *>>,
																	generic_copy_constructor<Resource<Platform::Shader *>>,
																	"Shader"};

	template <>
	inline constexpr Type get_type<Resource<Platform::Mesh *>> = {PrimitiveType::MESH_RESOURCE,
																  []() { return sizeof(Resource<Platform::Mesh *>); },
																  generic_type_serializer<Resource<Platform::Mesh *>>,
																  generic_type_deserializer<Resource<Platform::Mesh *>>,
																  generic_copy_constructor<Resource<Platform::Mesh *>>,
																  "Mesh"};

	template <>
	struct Traits<Platform::GraphicsPipelineInfo> : GenericTraits<Platform::GraphicsPipelineInfo>
	{
		static u64 hash(const Platform::GraphicsPipelineInfo &info)
		{
			// TODO(Brandon): This is rather hacky, and if we add more fields to the pipeline info maintaining this will be a pain in the ass.
			byte data[sizeof(bool) * 2 + sizeof(Platform::DepthTest) + sizeof(Platform::ShaderUsage)]{};
			memcpy(data, &info.write_depth, sizeof(info.write_depth));

			bool depth_test_has_value = info.depth_test.has_value();

			memcpy(data + sizeof(info.write_depth), &depth_test_has_value, sizeof(depth_test_has_value));

			if (depth_test_has_value)
				memcpy(data + sizeof(info.write_depth) + sizeof(depth_test_has_value), &info.depth_test.value(), sizeof(info.depth_test.value()));

			memcpy(data + sizeof(info.write_depth) + sizeof(depth_test_has_value) + sizeof(info.depth_test.value()), &info.shader_usage, sizeof(info.shader_usage));

			return string_hash((const char *)(data), sizeof(info));
		}
	};
} // namespace Vultr
