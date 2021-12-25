#include <render/types/vertex.h>
#include <render/types/vertex_buffer.h>
#include <glad/glad.h>

namespace Vultr
{
	VertexBuffer new_vertex_buffer(const GLvoid *vertices, size_t count)
	{
		VertexBuffer vbo;

		// Create and set the vertex buffer data
		glGenBuffers(1, &vbo);
		bind_vertex_buffer(vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * count, vertices, GL_STATIC_DRAW);
		return vbo;
	}

	VertexBuffer new_vertex_buffer(u32 size)
	{
		VertexBuffer vbo;

		glGenBuffers(1, &vbo);
		bind_vertex_buffer(vbo);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		return vbo;
	}

	void delete_vertex_buffer(VertexBuffer vbo) { glDeleteBuffers(1, &vbo); }

	void bind_vertex_buffer(VertexBuffer vbo) { glBindBuffer(GL_ARRAY_BUFFER, vbo); }

	void unbind_vertex_buffer() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
} // namespace Vultr
