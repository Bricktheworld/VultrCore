#pragma once
#include <vultr.h>
#include <platform/platform.h>
#include <filesystem/filesystem.h>

namespace Vultr
{
	struct Project
	{
		Platform::DLL dll{};
		Path resource_dir{};
		Path build_dir{};
		Path build_resource_dir{};
		VultrInitApi init{};
		VultrUpdateApi update{};
		VultrDestroyApi destroy{};
	};

	enum struct ResourceType
	{
		TEXTURE,
		SHADER,
		MATERIAL,
		MESH,
		CPP_SRC,
		SCENE,
		OTHER,
	};

	ResourceType get_resource_type(const Path &path);
	Path get_texture_resource(const Project *project, const Path &local);
	Tuple<Path, Path> get_shader_resource(const Project *project, const Path &local);
	Path get_material_resource(const Project *project, const Path &local);
	Tuple<Path, Path> get_mesh_resource(const Project *project, const Path &local);
	Path get_scene_resource(const Project *project, const Path &local);
	ErrorOr<Project> load_game(const Path &build_dir, const Path &resource_dir);
	ErrorOr<void> import_resource_dir(Project *project);
} // namespace Vultr