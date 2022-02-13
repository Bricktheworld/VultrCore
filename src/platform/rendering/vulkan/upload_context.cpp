#include "upload_context.h"
#include "render_context.h"

namespace Vultr
{
	namespace Platform
	{

		UploadContext *init_upload_context(RenderContext *c)
		{
			auto *upload_context   = v_alloc<UploadContext>();
			upload_context->device = Vulkan::get_device(c);
			return upload_context;
		}

		void destroy_upload_context(RenderContext *c, UploadContext *upload_context) { v_free(upload_context); }
	} // namespace Platform

	namespace Vulkan
	{
		Device *get_device(Platform::UploadContext *c) { return c->device; }
	} // namespace Vulkan
} // namespace Vultr
