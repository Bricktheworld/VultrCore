#pragma once
#include <vultr.h>
#include <platform/platform.h>
#include <filesystem/filesystem.h>

namespace Vultr
{
	enum struct ResourceType : u8
	{
		TEXTURE  = 0x0,
		SHADER   = 0x1,
		MATERIAL = 0x2,
		MESH     = 0x4,
		CPP_SRC  = 0x8,
		SCENE    = 0x10,
		OTHER    = 0x20,
	};

	inline constexpr const char *resource_type_to_string(ResourceType type)
	{
		switch (type)
		{
			case ResourceType::TEXTURE:
				return "TEXTURE";
			case ResourceType::SHADER:
				return "SHADER";
			case ResourceType::MATERIAL:
				return "MATERIAL";
			case ResourceType::MESH:
				return "MESH";
			case ResourceType::CPP_SRC:
				return "CPP_SRC";
			case ResourceType::SCENE:
				return "SCENE";
			case ResourceType::OTHER:
				return "OTHER";
		}
	}

	inline ResourceType resource_type_from_string(StringView string)
	{
		if (string == "TEXTURE")
			return ResourceType::TEXTURE;
		else if (string == "SHADER")
			return ResourceType::SHADER;
		else if (string == "MATERIAL")
			return ResourceType::MATERIAL;
		else if (string == "MESH")
			return ResourceType::MESH;
		else if (string == "CPP_SRC")
			return ResourceType::CPP_SRC;
		else if (string == "SCENE")
			return ResourceType::SCENE;
		else
			return ResourceType::OTHER;
	}

	struct Project
	{
		Path dll_location{};
		u32 index = 0;
		Platform::DLL dll{};

		Path resource_dir{};
		Path build_dir{};
		Path build_resource_dir{};
		Hashmap<UUID, Path> asset_map{};
		Queue<Tuple<UUID, ResourceType>, 1024> asset_reload_queue{};
		Platform::Mutex shader_free_mutex{};

		VultrRegisterComponentsApi register_components{};
		VultrInitApi init{};
		VultrUpdateApi update{};
		VultrDestroyApi destroy{};
	};

	ErrorOr<void> load_game(const Path &build_dir, const Path &resource_dir, Project *out);
	ErrorOr<void> reload_game(Project *project);
	void reload_necessary_assets(Project *project);

	struct MetadataHeader
	{
		u8 version                 = 1;
		ResourceType resource_type = ResourceType::TEXTURE;
		UUID uuid                  = Platform::generate_uuid();
	};

	struct TextureMetadata
	{
	};

	struct MeshMetadata
	{
	};

	struct SceneMetadata
	{
	};

	ResourceType get_resource_type(const Path &path);

	ErrorOr<MetadataHeader> get_resource_metadata(const Path &path);
	ErrorOr<MetadataHeader> get_or_create_resource_metadata(const Path &path);

	ErrorOr<TextureMetadata> get_texture_metadata(const Path &path);
	ErrorOr<MeshMetadata> get_mesh_metadata(const Path &path);
	ErrorOr<SceneMetadata> get_scene_metadata(const Path &path);

	ErrorOr<void> save_metadata(const Path &path, const MetadataHeader &metadata);

	ErrorOr<void> save_texture_metadata(const Path &path, const MetadataHeader &header, const TextureMetadata &metadata);
	ErrorOr<void> save_mesh_metadata(const Path &path, const MetadataHeader &header, const TextureMetadata &metadata);
	ErrorOr<void> save_scene_metadata(const Path &path, const MetadataHeader &header, const TextureMetadata &metadata);

	Path get_editor_optimized_path(const Project *project, const UUID &uuid);

	bool is_asset_imported(const Project *project, const Path &path);
	bool needs_reimport(const Project *project, const Path &src_file);

	ErrorOr<void> import_dir(Project *project, const Path &in_dir, atomic_u32 *progress);

	struct __attribute__((packed)) TextureBuildHeader
	{
		u8 version = 1;
		u32 width  = 0;
		u32 height = 0;
	};

	void build_editor_optimized_texture(Buffer *out, const Platform::ImportedTexture &imported);
	ErrorOr<Platform::Texture *> load_editor_optimized_texture(Platform::UploadContext *c, const Buffer &buffer);

	struct __attribute__((packed)) ShaderBuildHeader
	{
		u8 version    = 1;
		u64 vert_size = 0;
		u64 frag_size = 0;
	};

	void build_editor_optimized_shader(Buffer *out, const Platform::CompiledShaderSrc &compiled);
	ErrorOr<Platform::Shader *> load_editor_optimized_shader(Platform::RenderContext *c, const Buffer &buffer);

	struct __attribute__((packed)) MeshBuildHeader
	{
		u8 version       = 1;
		u64 vertex_count = 0;
		u64 index_count  = 0;
	};

	void build_editor_optimized_mesh(Buffer *out, const Platform::ImportedMesh &imported);
	ErrorOr<Platform::Mesh *> load_editor_optimized_mesh(Platform::UploadContext *c, const Buffer &buffer);

} // namespace Vultr