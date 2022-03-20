#include "project.h"
#include <platform/platform.h>

namespace Vultr
{
	ErrorOr<Project> load_game(const Path &build_dir, const Path &resource_dir)
	{
		Project project{};
		auto dll_location    = build_dir / VULTR_GAMEPLAY_NAME;
		project.build_dir    = build_dir;
		project.resource_dir = resource_dir;
		printf("Opening %s\n", dll_location.c_str());
		TRY_UNWRAP(project.dll, Platform::dl_open(dll_location.c_str()));

		TRY_UNWRAP(auto use_game_memory, Platform::dl_load_symbol<UseGameMemoryApi>(&project.dll, USE_GAME_MEMORY_SYMBOL));
		use_game_memory(g_game_memory);

		TRY_UNWRAP(project.init, Platform::dl_load_symbol<VultrInitApi>(&project.dll, VULTR_INIT_SYMBOL));
		TRY_UNWRAP(project.update, Platform::dl_load_symbol<VultrUpdateApi>(&project.dll, VULTR_UPDATE_SYMBOL));

		return project;
	}

	static bool needs_reimport(const Path &src_file, const Path &out_file)
	{
		if (!exists(out_file))
			return true;
		return fget_date_modified_ms(src_file).value() > fget_date_modified_ms(out_file).value();
	}

	static ErrorOr<void> import_resource_file(const Path &out_dir, const Path &local_src, const Path &full_src)
	{
		if let (auto extension, local_src.get_extension())
		{
			if (extension == ".fbx")
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
			}
			else if (extension == ".png")
			{
				return Success;
			}
			else
			{
				return Error("Invalid extension " + extension);
			}
			return Success;
		}
		else
		{
			return Error("No extension for file " + full_src.string());
		}
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
