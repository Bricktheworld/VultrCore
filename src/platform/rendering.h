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
			DEPTH,
		};

		inline u32 get_pixel_size(TextureFormat format)
		{
			switch (format)
			{
				case TextureFormat::RGB8:
					return 3 * 2;
				case TextureFormat::RGB16:
					return 3;
				case TextureFormat::RGBA8:
					return 4;
				case TextureFormat::RGBA16:
					return 4 * 2;
				case TextureFormat::SRGB8:
					return 3;
				case TextureFormat::SRGBA8:
					return 4;
				case TextureFormat::DEPTH:
					return 1;
			}
		}

		Texture *init_texture(RenderContext *c, u32 width, u32 height, TextureFormat format);
		Texture *init_texture(UploadContext *c, u32 width, u32 height, TextureFormat format);

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
		enum struct SamplerType
		{
			ALBEDO,
			NORMAL,
			METALLIC,
			ROUGHNESS,
			AMBIENT_OCCLUSION,
		};
		struct SamplerBinding
		{
			String name{};
			SamplerType type = SamplerType::ALBEDO;
		};
		struct ShaderReflection
		{
			Vector<UniformMember> uniform_members{};
			u32 uniform_size = 0;
			Vector<SamplerBinding> samplers{};
		};
		struct CompiledShaderSrc
		{
			Buffer vert_src{};
			Buffer frag_src{};
		};

		struct DescriptorSet;

		DescriptorSet *alloc_descriptor_set(UploadContext *c, Shader *shader);
		void free_descriptor_set(RenderContext *c, DescriptorSet *set);

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
		//		inline ErrorOr<void> export_mesh(const Path &vertex_output, const Path &index_output, const ImportedMesh *mesh)
		//		{
		//			TRY(try_fwrite_all(vertex_output, (byte *)mesh->vertices, sizeof(Vertex) * mesh->vertex_count, StreamWriteMode::OVERWRITE));
		//			TRY(try_fwrite_all(index_output, (byte *)mesh->indices, sizeof(u16) * mesh->index_count, StreamWriteMode::OVERWRITE));
		//			return Success;
		//		}

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
			Vec4 color{};
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

		struct GraphicsPipelineInfo
		{
			Shader *shader                = nullptr;
			VertexDescription description = get_vertex_description<Vertex>();
			// TODO(Brandon): Make this actually used.
			Option<DepthTest> depth_test = None;
		};

		struct GraphicsPipeline;
		GraphicsPipeline *init_pipeline(RenderContext *c, const GraphicsPipelineInfo &info);
		GraphicsPipeline *init_pipeline(RenderContext *c, Framebuffer *framebuffer, const GraphicsPipelineInfo &info);
		void destroy_pipeline(RenderContext *c, GraphicsPipeline *pipeline);
		void attach_pipeline(RenderContext *c, Framebuffer *framebuffer, const GraphicsPipelineInfo &info, u32 id);
		void remove_pipeline(RenderContext *c, Framebuffer *framebuffer, u32 id);

		struct CmdBuffer;

		ErrorOr<CmdBuffer *> begin_cmd_buffer(const Window *window);

		void begin_window_framebuffer(CmdBuffer *cmd, Vec4 clear_color = Vec4(0, 0, 0, 1));
		void begin_framebuffer(CmdBuffer *cmd, Framebuffer *framebuffer, Vec4 clear_color = Vec4(0, 0, 0, 1));
		void end_framebuffer(CmdBuffer *cmd);

		void bind_pipeline(CmdBuffer *cmd, GraphicsPipeline *pipeline);
		void bind_vertex_buffer(CmdBuffer *cmd, VertexBuffer *vbo);
		void bind_index_buffer(CmdBuffer *cmd, IndexBuffer *ibo);
		void draw_indexed(CmdBuffer *cmd, u32 index_count, u32 instance_count = 1, u32 first_index = 0, s32 vertex_offset = 0, u32 first_instance_id = 0);
		void push_constants(CmdBuffer *cmd, GraphicsPipeline *pipeline, const PushConstant &constant);
		void update_descriptor_set(DescriptorSet *set, void *data);
		void update_descriptor_set(DescriptorSet *set, const Option<ResourceId> &texture, u32 binding);
		void update_default_descriptor_set(CmdBuffer *cmd, CameraUBO *camera_ubo, DirectionalLightUBO *directional_light_ubo);
		void bind_descriptor_set(CmdBuffer *cmd, GraphicsPipeline *pipeline, DescriptorSet *set);
		void bind_material(CmdBuffer *cmd, GraphicsPipeline *pipeline, Material *material);

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
} // namespace Vultr
