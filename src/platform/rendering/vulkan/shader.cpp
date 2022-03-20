#include "shader.h"
#include "render_context.h"
#include "constants.h"
#include <vulkan/vulkan.h>

namespace Vultr
{
	namespace Platform
	{
		ErrorOr<Shader *> try_load_shader(RenderContext *c, const CompiledShaderSrc &compiled_shader)
		{
			auto *shader = v_alloc<Shader>();
			VkShaderModuleCreateInfo create_info{
				.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = compiled_shader.vert_src.size(),
				.pCode    = reinterpret_cast<const u32 *>(compiled_shader.vert_src.storage),
			};

			VK_TRY(vkCreateShaderModule(Vulkan::get_device(c)->device, &create_info, nullptr, &shader->vert_module))

			create_info = {
				.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = compiled_shader.frag_src.size(),
				.pCode    = reinterpret_cast<const u32 *>(compiled_shader.frag_src.storage),
			};

			VK_TRY(vkCreateShaderModule(Vulkan::get_device(c)->device, &create_info, nullptr, &shader->frag_module))

			return shader;
		}

		void destroy_shader(RenderContext *c, Shader *shader)
		{
			ASSERT(shader->vert_module != nullptr && shader->frag_module != nullptr, "Cannot destroy shader with module nullptr");
			auto *d = Vulkan::get_device(c);
			vkDestroyShaderModule(d->device, shader->vert_module, nullptr);
			vkDestroyShaderModule(d->device, shader->frag_module, nullptr);
			shader->vert_module = nullptr;
			shader->frag_module = nullptr;
			v_free(shader);
		}
	} // namespace Platform
} // namespace Vultr
