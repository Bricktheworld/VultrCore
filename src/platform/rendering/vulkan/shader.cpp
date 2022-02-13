#include "shader.h"
#include "render_context.h"
#include "constants.h"
#include <vulkan/vulkan.h>

namespace Vultr
{
	namespace Platform
	{
		ErrorOr<Shader> try_load_shader(RenderContext *c, Buffer src, ShaderType type)
		{
			VkShaderModuleCreateInfo create_info{
				.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = src.size(),
				.pCode    = reinterpret_cast<const u32 *>(src.storage),
			};

			VkShaderModule shader_module;
			VK_TRY(vkCreateShaderModule(Vulkan::get_device(c)->device, &create_info, nullptr, &shader_module))

			return Shader{
				.type      = type,
				.vk_module = shader_module,
			};
		}

		void destroy_shader(RenderContext *c, Shader *shader)
		{
			ASSERT(shader->vk_module != nullptr, "Cannot destroy shader with module nullptr");
			vkDestroyShaderModule(Vulkan::get_device(c)->device, shader->vk_module, nullptr);
			shader->vk_module = nullptr;
		}
	} // namespace Platform
} // namespace Vultr
