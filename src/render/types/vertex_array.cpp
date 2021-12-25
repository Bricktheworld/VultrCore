#include <render/types/vertex_array.h>
#include <glad/glad.h>

namespace Vultr
{
	VertexArray new_vertex_array()
	{
		VertexArray vao;
		glGenVertexArrays(1, &vao);
		return vao;
	}

	void delete_vertex_array(VertexArray vao) { glDeleteVertexArrays(1, &vao); }

	void bind_vertex_array(VertexArray vao) { glBindVertexArray(vao); }

	void unbind_vertex_array() { glBindVertexArray(0); }
} // namespace Vultr
