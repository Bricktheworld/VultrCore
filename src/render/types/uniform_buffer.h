#pragma once
#include <types/types.h>
#include "shader.h"

namespace Vultr
{
	struct UniformBuffer
	{
		u32 id            = 0;
		u16 binding_point = 0;
		char *label       = nullptr;
	};

	bool is_valid_uniform_buffer(const UniformBuffer &ubo);

	UniformBuffer invalid_uniform_buffer();

	UniformBuffer new_uniform_buffer(const char *label, u16 binding_point, size_t size);

	void delete_uniform_buffer(const UniformBuffer &ubo);

	void bind_uniform_buffer(const UniformBuffer &ubo);

	void unbind_all_uniform_buffers();

	void attach_shader_uniform_buffer(Shader *shader, UniformBuffer &ubo);
} // namespace Vultr
