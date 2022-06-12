#pragma once
#include <vultr.h>

namespace Vultr
{
	struct ResourceFile
	{
		Path path{};
		UUID uuid{};
		MetadataHeader metadata{};
		Option<Platform::Framebuffer *> rendered_framebuffer   = None;
		Resource<Platform::Texture *> resource_texture{};
		Resource<Platform::Material *> resource_material{};
		Resource<Platform::Mesh *> resource_mesh{};

		~ResourceFile()
		{
			if let (auto fbo, rendered_framebuffer)
			{
				Platform::destroy_framebuffer(engine()->context, fbo);
			}
			else
			{
			}
		}
	};

	struct ResourceBrowser
	{
		Vector<ResourceFile> files{};
		Vector<Path> dirs{};
		Path current_dir{};
		Option<u32> selected_index                = None;
		atomic_bool need_refresh                  = true;

		Platform::Mesh *material_sphere           = nullptr;

		Platform::Shader *mesh_shader             = nullptr;

		Input::CallbackHandle key_callback{};
	};
} // namespace Vultr