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
		project.build_resource_dir = build_dir / "res";
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

	static ErrorOr<void> import_resource_file(const Path &out_dir, const Path &local_src, const Path &full_src)
	{
		switch (get_resource_type(local_src))
		{
			case ResourceType::MESH:
			{
				auto vertex_out = out_dir / (local_src.basename() + ".vertex");
				auto index_out  = out_dir / (local_src.basename() + ".index");
				if (needs_reimport(full_src, vertex_out) || needs_reimport(full_src, index_out))
				{
					printf("Importing mesh %s\n", full_src.c_str());
					TRY_UNWRAP(auto mesh, Platform::import_mesh_file(full_src));
					TRY(Platform::export_mesh(vertex_out, index_out, &mesh));
					Platform::free_imported_mesh(&mesh);
				}
				break;
			}
			case ResourceType::SHADER:
			{
				auto vertex_out   = out_dir / (local_src.basename() + ".vert_spv");
				auto fragment_out = out_dir / (local_src.basename() + ".frag_spv");
				if (needs_reimport(full_src, vertex_out) || needs_reimport(full_src, fragment_out))
				{
					String src{};
					TRY(try_fread_all(full_src, &src));
					TRY_UNWRAP(auto compiled, Platform::try_compile_shader(src));
					TRY(try_fwrite_all(vertex_out, compiled.vert_src, StreamWriteMode::OVERWRITE));
					TRY(try_fwrite_all(fragment_out, compiled.frag_src, StreamWriteMode::OVERWRITE));
				}
				break;
			}
			case ResourceType::TEXTURE:
			{
				auto out = out_dir / (local_src.basename() + ".texture");
				if (needs_reimport(full_src, out))
				{
					Buffer imported{};
					TRY(Platform::import_texture_file(full_src, &imported));
					TRY(try_fwrite_all(out, imported, StreamWriteMode::OVERWRITE));
				}
				break;
			}
			default:
			{
				auto out = out_dir / local_src.basename();
				if (needs_reimport(full_src, out))
				{
					Buffer src{};
					TRY(try_fread_all(full_src, &src));
					TRY(try_fwrite_all(out, src, StreamWriteMode::OVERWRITE));
				}
				break;
			}
		}
		return Success;
	}

	static ErrorOr<void> import_dir(const Path &in_dir, const Path &out_dir)
	{
		for (auto entry : DirectoryIterator(in_dir))
		{
			auto local = Path(entry.string().substr(in_dir.string().length()));
			if (entry.is_directory())
			{
				auto entry_dir = out_dir / local;
				TRY(makedir(entry_dir));
				TRY(import_dir(entry, entry_dir));
			}
			else
			{
				TRY(import_resource_file(out_dir, local, entry))
			}
		}
		return Success;
	}

	ErrorOr<void> import_resource_dir(Project *project)
	{
		auto resource_dir = project->build_dir / "res/";
		TRY(makedir(resource_dir));

		printf("Created build resource directory %s\n", resource_dir.c_str());

		TRY(import_dir(project->resource_dir, resource_dir));

		return Success;
	}
} // namespace Vultr
