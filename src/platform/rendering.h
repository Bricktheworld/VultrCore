#pragma once
#include "platform.h"
#include <types/vector.h>
#include <types/buffer.h>
#include <glm/glm.hpp>
#include <filesystem/path.h>

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
		struct TextureRef;

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

		Texture *init_texture(RenderContext *c, u32 width, u32 height, TextureFormat format);
		void destroy_texture(RenderContext *c, Texture *texture);

		struct RenderPassInfo
		{
		};

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

			LoadOp load_op       = LoadOp::DONT_CARE;
			StoreOp store_op     = StoreOp::STORE;
		};

		Framebuffer *init_framebuffer(RenderContext *c, const Vector<AttachmentDescription> &attachments, Option<u32> width = None, Option<u32> height = None);
		void destroy_framebuffer(RenderContext *c, Framebuffer *framebuffer);

		struct Shader;

		Shader *init_shader();

		enum struct ShaderType : u8
		{
			VERT = 0x1,
			FRAG = 0x2,
		};

		ErrorOr<Shader *> try_load_shader(RenderContext *c, Buffer src, ShaderType type);
		void destroy_shader(RenderContext *c, Shader *shader);

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
		void destroy_vertex_buffer(UploadContext *c, VertexBuffer *buffer);

		IndexBuffer *init_index_buffer(UploadContext *c, u16 *data, size_t size);
		void destroy_index_buffer(UploadContext *c, IndexBuffer *buffer);

		inline Mesh *init_mesh(UploadContext *c, Vertex *vertices, size_t vertex_count, u16 *indices, size_t index_count)
		{
			auto *mesh         = v_alloc<Mesh>();

			mesh->vertices     = v_alloc<Vertex>(vertex_count);
			mesh->vertex_count = vertex_count;
			Utils::copy(mesh->vertices, vertices, vertex_count);

			mesh->vertex_buffer = init_vertex_buffer(c, vertices, sizeof(Vertex) * vertex_count);

			mesh->indices       = v_alloc<u16>(index_count);
			mesh->index_count   = index_count;
			Utils::copy(mesh->indices, indices, index_count);

			mesh->index_buffer = init_index_buffer(c, indices, sizeof(u16) * index_count);
			return mesh;
		}

		ErrorOr<Mesh *> load_mesh_file(UploadContext *c, const Path &path);
		ErrorOr<Mesh *> load_mesh_memory(UploadContext *c, byte *data, u64 size);

		inline void destroy_mesh(UploadContext *c, Mesh *mesh)
		{
			destroy_index_buffer(c, mesh->index_buffer);
			destroy_vertex_buffer(c, mesh->vertex_buffer);
			v_free(mesh->vertices);
			v_free(mesh->indices);
			v_free(mesh);
		}

		struct PushConstant
		{
			Vec4 color{};
			Mat4 model{};
		};

		enum struct DescriptorSetBindingType
		{
			UNIFORM_BUFFER,
			TEXTURE
		};

		struct DescriptorSetBinding
		{
			DescriptorSetBindingType type = DescriptorSetBindingType::UNIFORM_BUFFER;
			u32 size                      = 0;
		};

		struct DescriptorLayout;

		template <size_t size, DescriptorSetBindingType type>
		struct BindingT
		{
			static constexpr DescriptorSetBinding get_binding()
			{
				return {
					.type = type,
					.size = size,
				};
			}
		};

		template <typename T>
		struct UboBinding : BindingT<sizeof(T), DescriptorSetBindingType::UNIFORM_BUFFER>
		{
		};

		DescriptorLayout *init_descriptor_layout(RenderContext *c, const Vector<DescriptorSetBinding> &bindings, u32 max_objects);

		template <typename... T>
		DescriptorLayout *init_descriptor_layout(RenderContext *c, u32 max_objects)
		{
			return init_descriptor_layout(c, Vector<DescriptorSetBinding>({T::get_binding()...}), max_objects);
		}
		void destroy_descriptor_layout(RenderContext *c, DescriptorLayout *layout);
		void register_descriptor_layout(RenderContext *c, DescriptorLayout *layout);

		struct GraphicsPipelineInfo
		{
			Shader *vert                  = nullptr;
			Shader *frag                  = nullptr;
			VertexDescription description = get_vertex_description<Vertex>();
			Vector<DescriptorLayout *> descriptor_layouts{};
		};

		struct GraphicsPipeline;
		GraphicsPipeline *init_pipeline(RenderContext *c, const GraphicsPipelineInfo &info);
		void destroy_pipeline(RenderContext *c, GraphicsPipeline *pipeline);

		struct CmdBuffer;

		ErrorOr<CmdBuffer *> begin_cmd_buffer(const Window *window);
		//		void cmd_begin_renderpass(CmdBuffer *cmd);

		void bind_pipeline(CmdBuffer *cmd, GraphicsPipeline *pipeline);
		void bind_vertex_buffer(CmdBuffer *cmd, VertexBuffer *vbo);
		void bind_index_buffer(CmdBuffer *cmd, IndexBuffer *ibo);
		void draw_indexed(CmdBuffer *cmd, u32 index_count, u32 instance_count = 1, u32 first_index = 0, s32 vertex_offset = 0, u32 first_instance_id = 0);
		void push_constants(CmdBuffer *cmd, GraphicsPipeline *pipeline, const PushConstant &constant);
		void update_descriptor_set(CmdBuffer *cmd, DescriptorLayout *layout, void *data, u32 index, u32 binding);
		void flush_descriptor_set_changes(CmdBuffer *cmd);
		void bind_descriptor_set(CmdBuffer *cmd, GraphicsPipeline *pipeline, DescriptorLayout *layout, u32 set, u32 index);

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
		ImGuiContext *init_imgui(const Window *window, UploadContext *upload_context);
		void imgui_begin_frame(CmdBuffer *cmd, ImGuiContext *c);
		void imgui_end_frame(CmdBuffer *cmd, ImGuiContext *c);
		void destroy_imgui(RenderContext *c, ImGuiContext *imgui_c);
	} // namespace Platform
} // namespace Vultr
