#include "../../rendering.h"
#include "command_pool.h"

namespace Vultr
{
	namespace Platform
	{
		void draw_indexed(CmdBuffer *cmd, u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance_id)
		{
			vkCmdDrawIndexed(cmd->cmd_buffer, index_count, instance_count, first_index, vertex_offset, first_instance_id);
		}
	} // namespace Platform
} // namespace Vultr
