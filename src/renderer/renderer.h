#pragma once
#include <types/types.h>
#include "mesh.h"
#include "framebuffer.h"
#include "shader.h"
#include "texture.h"
#include "render_command.h"

namespace Vultr
{
#define MAX_RENDER_COMMAND_BUFFER 6000
	struct RenderCommandBuffer
	{
		RenderCommand buffer[MAX_RENDER_COMMAND_BUFFER];
	};

	struct Renderer
	{
		u8 buffer                              = 0;
		RenderCommandBuffer command_buffers[2] = {RenderCommandBuffer{}, RenderCommandBuffer{}};
	};
} // namespace Vultr
