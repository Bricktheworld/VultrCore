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
		// void draw_frame(const Window *window, RenderContext *c, f64 delta_time);
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

		template <Vertex>
		inline VertexDescription get_vertex_description()
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
				}),
			};
		}

		struct Framebuffer;
		struct Shader;
		enum struct ShaderType
		{
			VERT,
			FRAG,
		};

		ErrorOr<Shader> try_load_shader(RenderContext *c, Buffer src, ShaderType type);
		void destroy_shader(RenderContext *c, Shader *shader);

		struct Texture;
		struct VertexBuffer;
		struct IndexBuffer;

		template <typename T>
		struct MeshT
		{
			T *vertices                 = nullptr;
			u16 *indices                = nullptr;
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
			auto *mesh     = v_alloc<Mesh>();

			mesh->vertices = v_alloc<Vertex>(vertex_count);
			Utils::copy(mesh->vertices, vertices, vertex_count);

			mesh->vertex_buffer = init_vertex_buffer(c, vertices, sizeof(Vertex) * vertex_count);

			mesh->indices       = v_alloc<u16>(index_count);
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
			mesh = {};
		}

		struct GraphicsPipelineInfo
		{
			Shader *vert                  = nullptr;
			Shader *frag                  = nullptr;
			VertexDescription description = get_vertex_description<Vertex>();
		};

		struct GraphicsPipeline;
		GraphicsPipeline *init_pipeline(RenderContext *c, const GraphicsPipelineInfo &info);
		void destroy_pipeline(RenderContext *c, GraphicsPipeline *pipeline);

		struct RenderPassInfo
		{
		};

		struct RenderPass;

		struct CmdBuffer;

		ErrorOr<CmdBuffer *> begin_cmd_buffer(const Window *window);
		void cmd_begin_renderpass(CmdBuffer *cmd);
		void end_cmd_buffer(CmdBuffer *c);
	} // namespace Platform
} // namespace Vultr
