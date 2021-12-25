#pragma once
#include <glm/glm.hpp>
#include <render/types/index_buffer.h>
#include <render/types/shader.h>
#include <render/types/vertex.h>
#include <render/types/vertex_array.h>
#include <render/types/vertex_buffer.h>

namespace Vultr
{
	struct Mesh
	{
		// Holds the saved vertices and indices in a buffer cpu side
		Vertex *vertices    = nullptr;
		size_t vertex_count = 0;
		u16 *indices        = nullptr;
		size_t index_count  = 0;

		// A bunch of required buffers
		VertexArray vao         = 0;
		IndexBuffer ibo         = 0;
		VertexBuffer vbo        = 0;

		Mesh()                  = default;
		Mesh(const Mesh &other) = delete;
	};

	bool is_valid_mesh(const Mesh *mesh);

	void new_mesh(Mesh *mesh, Vec3 positions[], Vec2 uvs[], Vec3 normals[], size_t vertex_count, u16 indices[], size_t index_count, bool load_gpu);
	void new_mesh(Mesh *mesh, Vertex vertices[], size_t vertex_count, u16 indices[], size_t index_count, bool load_gpu);
	void mesh_init_gpu(Mesh *mesh);

	void delete_mesh(Mesh *mesh);

	void draw_mesh(const Mesh *mesh);
} // namespace Vultr
