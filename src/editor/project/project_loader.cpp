#include "project.h"
#include <platform/platform.h>

namespace Vultr
{
	ErrorOr<Project> load_game(const Path &build_dir, const Path &resource_dir)
	{
		Project project{};
		auto dll_location          = build_dir / VULTR_GAMEPLAY_NAME;
		project.build_dir          = build_dir;
		project.resource_dir       = resource_dir;
		project.build_resource_dir = build_dir / "res/";
		printf("Opening %s\n", dll_location.c_str());
		TRY_UNWRAP(project.dll, Platform::dl_open(dll_location.c_str()));

		TRY_UNWRAP(auto use_game_memory, Platform::dl_load_symbol<UseGameMemoryApi>(&project.dll, USE_GAME_MEMORY_SYMBOL));
		use_game_memory(g_game_memory);

		TRY_UNWRAP(project.init, Platform::dl_load_symbol<VultrInitApi>(&project.dll, VULTR_INIT_SYMBOL));
		TRY_UNWRAP(project.update, Platform::dl_load_symbol<VultrUpdateApi>(&project.dll, VULTR_UPDATE_SYMBOL));
		TRY_UNWRAP(project.destroy, Platform::dl_load_symbol<VultrDestroyApi>(&project.dll, VULTR_DESTROY_SYMBOL));

		return project;
	}

	static bool needs_reimport(const Path &src_file, const Path &out_file)
	{
		if (!exists(out_file))
			return true;
		return fget_date_modified_ms(src_file).value() > fget_date_modified_ms(out_file).value();
	}

	ResourceType get_resource_type(const Path &path)
	{
		if let (auto extension, path.get_extension())
		{
			if (extension == ".fbx" || extension == ".obj" || extension == ".blend")
			{
				return ResourceType::MESH;
			}
			else if (extension == ".glsl")
			{
				return ResourceType::SHADER;
			}
			else if (extension == ".png" || extension == ".jpg" || extension == ".bmp")
			{
				return ResourceType::TEXTURE;
			}
			else if (extension == ".mat")
			{
				return ResourceType::MATERIAL;
			}
			else if (extension == ".c" || extension == ".cpp" || extension == ".cc" || extension == ".h" || extension == ".hpp")
			{
				return ResourceType::CPP_SRC;
			}
			else if (extension == ".vultr")
			{
				return ResourceType::SCENE;
			}
			else
			{
				return ResourceType::OTHER;
			}
		}
		else
		{
			return ResourceType::OTHER;
		}
	}

	Path get_texture_resource(const Project *project, const Path &local) { return project->build_resource_dir / (local.string() + ".texture"); }
	Tuple<Path, Path> get_shader_resource(const Project *project, const Path &local)
	{
		return {project->build_resource_dir / (local.string() + ".vert_spv"), project->build_resource_dir / (local.string() + ".frag_spv")};
	}
	Path get_material_resource(const Project *project, const Path &local) { return project->build_resource_dir / local; }
	Tuple<Path, Path> get_mesh_resource(const Project *project, const Path &local) { return {project->build_resource_dir / (local.string() + ".vertex"), project->build_resource_dir / (local.string() + ".index")}; }
	Path get_scene_resource(const Project *project, const Path &local) { return project->build_resource_dir / local; }
} // namespace Vultr
