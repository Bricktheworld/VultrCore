#pragma once
#include <types/types.h>
#include <assert.h>
#include <glm/glm.hpp>
#include <render/types/vertex.h>

namespace Vultr
{
	typedef u32 VertexArray;

	VertexArray new_vertex_array();
	void delete_vertex_array(VertexArray vao);

	void bind_vertex_array(VertexArray vao);
	void unbind_vertex_array();

	template <typename T>
	void setup_vertex_array();
} // namespace Vultr
