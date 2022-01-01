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
		RenderCommandBuffer buffer;
	};
} // namespace Vultr
