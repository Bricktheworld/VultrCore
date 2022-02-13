#include "upload_context.h"
#include "render_context.h"

namespace Vultr
{
	namespace Platform
	{

		UploadContext *init_upload_context(RenderContext *c)
		{
			auto *upload_context     = v_alloc<UploadContext>();
			upload_context->device   = Vulkan::get_device(c);
			upload_context->cmd_pool = Vulkan::init_cmd_pool(Vulkan::get_device(c));
			return upload_context;
		}

		void destroy_upload_context(UploadContext *upload_context)
		{
			Vulkan::destroy_cmd_pool(Vulkan::get_device(upload_context), &upload_context->cmd_pool);
			upload_context->device = nullptr;
			v_free(upload_context);
		}
	} // namespace Platform

	namespace Vulkan
	{
		Device *get_device(Platform::UploadContext *c)
		{
			ASSERT(c->device != nullptr, "Cannot get device from destroyed upload context!");
			return c->device;
		}
	} // namespace Vulkan
} // namespace Vultr
