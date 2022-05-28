#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "device.h"
#include "swap_chain.h"
#include "gpu_buffer.h"
#include "pipeline.h"

namespace Vultr
{
	namespace Platform
	{
		struct RenderContext
		{
			Vulkan::SwapChain swap_chain{};
			Texture *black_texture  = nullptr;
			Texture *white_texture  = nullptr;
			Texture *normal_texture = nullptr;
			CmdBuffer cmd{};
		};

	} // namespace Platform

	namespace Vulkan
	{
		inline Vulkan::Device *get_device(Platform::RenderContext *c) { return &c->swap_chain.device; }
		inline Vulkan::Device *get_device(SwapChain *sc) { return &sc->device; }
		inline Vulkan::SwapChain *get_swapchain(Platform::RenderContext *c) { return &c->swap_chain; }
	} // namespace Vulkan

} // namespace Vultr
