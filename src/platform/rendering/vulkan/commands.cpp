#include "command_pool.h"

namespace Vultr
{
	namespace Platform
	{
		void draw_indexed(CmdBuffer *cmd, u32 index_count, u32 instance_count, u32 first_index, s32 vertex_offset, u32 first_instance_id)
		{
			vkCmdDrawIndexed(cmd->cmd_buffer, index_count, instance_count, first_index, vertex_offset, first_instance_id);
		}

		//		void push_constants(CmdBuffer *cmd, Material *material, const PushConstant &constant)
		//		{
		//			ASSERT(material->source.loaded(), "Cannot push constants before material shader has been loaded!");
		//			auto *pipeline = Vulkan::get_or_create_pipeline(cmd->render_context, material->source.value(), info, cmd->current_framebuffer);
		//			Vulkan::bind_pipeline(cmd, pipeline);
		//
		//			vkCmdPushConstants(cmd->cmd_buffer, pipeline->vk_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &constant);
		//		}
	} // namespace Platform
} // namespace Vultr
