#include "project.h"
#include <platform/platform.h>
#include <yaml-cpp/yaml.h>

namespace Vultr
{
	static ErrorOr<void> load_game_symbols(Project *project)
	{
		TRY(makedir(project->build_dir / "binaries"));
		project->dll_location = project->build_dir / ("binaries/libGameplay" + serialize_u64(project->index) + ".so");

		Buffer dll_buf;
		TRY(try_fread_all(project->build_dir / VULTR_GAMEPLAY_NAME, &dll_buf));
		TRY(try_fwrite_all(project->dll_location, dll_buf, StreamWriteMode::OVERWRITE));

		printf("Opening %s\n", project->dll_location.c_str());
		TRY_UNWRAP(project->dll, Platform::dl_open(project->dll_location.c_str()));

		TRY_UNWRAP(auto use_game_memory, Platform::dl_load_symbol<UseGameMemoryApi>(&project->dll, USE_GAME_MEMORY_SYMBOL));
		use_game_memory(g_game_memory);

		TRY_UNWRAP(project->register_components, Platform::dl_load_symbol<VultrRegisterComponentsApi>(&project->dll, VULTR_REGISTER_COMPONENTS_SYMBOL));
		TRY_UNWRAP(project->init, Platform::dl_load_symbol<VultrInitApi>(&project->dll, VULTR_INIT_SYMBOL));
		TRY_UNWRAP(project->update, Platform::dl_load_symbol<VultrUpdateApi>(&project->dll, VULTR_UPDATE_SYMBOL));
		TRY_UNWRAP(project->destroy, Platform::dl_load_symbol<VultrDestroyApi>(&project->dll, VULTR_DESTROY_SYMBOL));

		return Success;
	}

	static Path get_metadata_file(const Path &path) { return Path(path.string() + ".meta"); }

	ErrorOr<void> load_game(const Path &build_dir, const Path &resource_dir, Project *out)
	{
		out->build_dir          = build_dir;
		out->resource_dir       = resource_dir;
		out->build_resource_dir = build_dir / "res/";
		out->index              = 0;
		TRY(load_game_symbols(out));
		return Success;
	}

	ErrorOr<void> reload_game(Project *project)
	{
		Platform::dl_close(&project->dll);
		project->index++;
		TRY(load_game_symbols(project));
		return Success;
	}

	void reload_necessary_assets(Project *project)
	{
		while (!project->asset_reload_queue.empty())
		{
			auto [uuid, type] = project->asset_reload_queue.pop();
			switch (type)
			{
				case ResourceType::TEXTURE:
				{
					resource_allocator<Platform::Texture *>()->notify_reload(uuid);
					break;
				}
				case ResourceType::SHADER:
				{
					Platform::Lock l(project->shader_free_mutex);
					resource_allocator<Platform::Shader *>()->notify_reload(uuid);
					break;
				}
				case ResourceType::MATERIAL:
				{
					resource_allocator<Platform::Material *>()->notify_reload(uuid);
					break;
				}
				case ResourceType::MESH:
				{
					resource_allocator<Platform::Mesh *>()->notify_reload(uuid);
					break;
				}
				default:
				{
					THROW("Invalid resource to reload!");
				}
			}
		}
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
			else if (extension == ".compute")
			{
				return ResourceType::COMPUTE_SHADER;
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

	ErrorOr<MetadataHeader> get_resource_metadata(const Path &path)
	{
		auto meta_path = get_metadata_file(path);

		if (!exists(meta_path))
			return Error("Metadata file does not exist");

		String meta_file{};

		TRY(try_fread_all(meta_path, &meta_file));

		MetadataHeader header{};

		auto root = YAML::Load(meta_file);
		if (!root.IsMap())
			return Error("Invalid meta format!");

		if (root["Version"])
			header.version = root["Version"].as<int>();
		else
			header.version = 1;

		if (root["ResourceType"])
			header.resource_type = static_cast<ResourceType>(root["ResourceType"].as<int>());
		else
			return Error("No resource type found!");

		if (root["UUID"])
			header.uuid = root["UUID"].as<UUID>();
		else
			return Error("No UUID found!");

		return header;
	}

	static ErrorOr<void> save_empty_metadata(const Path &path, const MetadataHeader &header)
	{
		switch (header.resource_type)
		{
			case ResourceType::TEXTURE:
				TRY(save_texture_metadata(path, header, {}));
				break;
			case ResourceType::SHADER:
				TRY(save_metadata(path, header));
				break;
			case ResourceType::COMPUTE_SHADER:
				TRY(save_metadata(path, header));
				break;
			case ResourceType::MATERIAL:
				TRY(save_metadata(path, header));
				break;
			case ResourceType::MESH:
				TRY(save_mesh_metadata(path, header, {}));
				break;
			case ResourceType::CPP_SRC:
				TRY(save_metadata(path, header));
				break;
			case ResourceType::SCENE:
				TRY(save_scene_metadata(path, header, {}));
				break;
			case ResourceType::OTHER:
				TRY(save_metadata(path, header));
				break;
		}
		return Success;
	}

	ErrorOr<MetadataHeader> get_or_create_resource_metadata(const Path &path)
	{
		if check (get_resource_metadata(path), auto header, auto err)
		{
			auto resource_type = get_resource_type(path);

			if (header.resource_type != resource_type)
			{
				header.resource_type = resource_type;
				save_empty_metadata(path, header);
			}

			return header;
		}
		else
		{
			MetadataHeader header{};
			header.resource_type = get_resource_type(path);
			save_empty_metadata(path, header);
			return header;
		}
	}

	ErrorOr<TextureMetadata> get_texture_metadata(const Path &path) { return TextureMetadata{}; }

	ErrorOr<MeshMetadata> get_mesh_metadata(const Path &path) { return MeshMetadata{}; }

	ErrorOr<SceneMetadata> get_scene_metadata(const Path &path) { return SceneMetadata{}; }

	YAML::Emitter &operator<<(YAML::Emitter &out, const MetadataHeader &m)
	{
		out << YAML::BeginMap;

		out << YAML::Key << "Version";
		out << YAML::Value << (int)m.version;

		out << YAML::Key << "ResourceType";
		out << YAML::Value << (int)static_cast<u8>(m.resource_type);

		out << YAML::Key << "UUID";
		out << YAML::Value << m.uuid;

		out << YAML::EndMap;
		return out;
	}

	ErrorOr<void> save_metadata(const Path &path, const MetadataHeader &metadata)
	{
		YAML::Emitter o{};
		o << metadata;
		auto output_stream = FileOutputStream(get_metadata_file(path), StreamFormat::UTF8, StreamWriteMode::OVERWRITE);
		TRY(output_stream.write(o.c_str(), o.size()));
		TRY(output_stream.write(String("\n")));
		return Success;
	}

	ErrorOr<void> save_texture_metadata(const Path &path, const MetadataHeader &header, const TextureMetadata &metadata)
	{
		YAML::Emitter o{};
		o << header;
		auto output_stream = FileOutputStream(get_metadata_file(path), StreamFormat::UTF8, StreamWriteMode::OVERWRITE);
		TRY(output_stream.write(o.c_str(), o.size()));
		TRY(output_stream.write(String("\n")));
		return Success;
	}

	ErrorOr<void> save_mesh_metadata(const Path &path, const MetadataHeader &header, const TextureMetadata &metadata)
	{
		YAML::Emitter o{};
		o << header;
		auto output_stream = FileOutputStream(get_metadata_file(path), StreamFormat::UTF8, StreamWriteMode::OVERWRITE);
		TRY(output_stream.write(o.c_str(), o.size()));
		TRY(output_stream.write(String("\n")));
		return Success;
	}

	ErrorOr<void> save_scene_metadata(const Path &path, const MetadataHeader &header, const TextureMetadata &metadata)
	{
		YAML::Emitter o{};
		o << header;
		auto output_stream = FileOutputStream(get_metadata_file(path), StreamFormat::UTF8, StreamWriteMode::OVERWRITE);
		TRY(output_stream.write(o.c_str(), o.size()));
		TRY(output_stream.write(String("\n")));
		return Success;
	}

	Path get_editor_optimized_path(const Project *project, const UUID &uuid)
	{
		Platform::UUID_String uuid_string;
		Platform::stringify_uuid(uuid, uuid_string);
		return project->build_resource_dir / uuid_string;
	}

	bool is_asset_imported(const Project *project, const Path &path)
	{
		if check (get_resource_metadata(path), auto metadata, auto _)
		{
			if (get_resource_type(path) != metadata.resource_type)
				return false;

			auto out_file = get_editor_optimized_path(project, metadata.uuid);
			if (!exists(out_file))
				return false;

			return true;
		}
		else
		{
			return false;
		}
	}

	bool needs_reimport(const Project *project, const Path &src_file)
	{
		if check (get_resource_metadata(src_file), auto metadata, auto _)
		{
			if (get_resource_type(src_file) != metadata.resource_type)
				return true;

			auto out_file = get_editor_optimized_path(project, metadata.uuid);
			if (!exists(out_file))
				return true;
			return fget_date_modified_ms(src_file).value() > fget_date_modified_ms(out_file).value();
		}
		else
		{
			return true;
		}
	}

	ErrorOr<void> import_dir(Project *project, const Path &in_dir, atomic_u32 *progress)
	{
		for (auto entry : DirectoryIterator(in_dir))
		{
			if (entry.is_directory())
			{
				TRY(import_dir(project, entry, progress));
			}
			else
			{
				if (entry.get_extension() == ".meta")
					continue;

				if (!needs_reimport(project, entry))
					continue;

				TRY_UNWRAP(auto metadata, get_or_create_resource_metadata(entry));
				auto out_path = get_editor_optimized_path(project, metadata.uuid);

				switch (metadata.resource_type)
				{
					case ResourceType::TEXTURE:
					{
						Platform::ImportedTexture texture{};
						TRY(Platform::import_texture_file(entry, &texture));

						Buffer imported{};
						build_editor_optimized_texture(&imported, texture);

						TRY(try_fwrite_all(out_path, imported, StreamWriteMode::OVERWRITE));

						if (ResourceId(metadata.uuid).loaded<Platform::Texture *>())
							project->asset_reload_queue.push({metadata.uuid, ResourceType::TEXTURE});

						break;
					}
					case ResourceType::SHADER:
					{
						String src{};
						TRY(try_fread_all(entry, &src));
						TRY_UNWRAP(auto shader, Platform::try_compile_shader(src));

						Buffer imported{};
						build_editor_optimized_shader(&imported, shader);

						TRY(try_fwrite_all(out_path, imported, StreamWriteMode::OVERWRITE));
						if (ResourceId(metadata.uuid).loaded<Platform::Shader *>())
							project->asset_reload_queue.push({metadata.uuid, ResourceType::SHADER});

						break;
					}
					case ResourceType::COMPUTE_SHADER:
					{
						String src{};
						TRY(try_fread_all(entry, &src));
						TRY_UNWRAP(auto compute_shader, Platform::try_compile_compute_shader(src));

						Buffer imported{};
						build_editor_optimized_compute_shader(&imported, compute_shader);

						TRY(try_fwrite_all(out_path, imported, StreamWriteMode::OVERWRITE));

						break;
					}
					case ResourceType::MESH:
					{
						TRY_UNWRAP(auto mesh, Platform::import_mesh_file(entry));

						Buffer imported;
						build_editor_optimized_mesh(&imported, mesh);

						TRY(try_fwrite_all(out_path, imported, StreamWriteMode::OVERWRITE));

						Platform::free_imported_mesh(&mesh);
						if (ResourceId(metadata.uuid).loaded<Platform::Mesh *>())
							project->asset_reload_queue.push({metadata.uuid, ResourceType::MESH});

						break;
					}
					case ResourceType::MATERIAL:
					{
						Buffer src{};
						TRY(try_fread_all(entry, &src));
						TRY(try_fwrite_all(out_path, src, StreamWriteMode::OVERWRITE));
						if (ResourceId(metadata.uuid).loaded<Platform::Material *>())
							project->asset_reload_queue.push({metadata.uuid, ResourceType::MATERIAL});

						break;
					}
					default:
					{
						Buffer src{};
						TRY(try_fread_all(entry, &src));
						TRY(try_fwrite_all(out_path, src, StreamWriteMode::OVERWRITE));
						break;
					}
				}
				(*progress)++;
				ftouch(entry);
			}
		}
		return Success;
	}

	void build_editor_optimized_texture(Buffer *out, const Platform::ImportedTexture &imported)
	{
		TextureBuildHeader header{};
		header.version   = 1;
		header.width     = imported.width;
		header.height    = imported.height;

		u16 header_size  = sizeof(header);
		u64 texture_size = imported.src.size();
		out->resize(header_size + texture_size);

		out->fill(reinterpret_cast<byte *>(&header), header_size, 0);
		out->fill(imported.src.storage, texture_size, header_size);
	}

	ErrorOr<Platform::Texture *> load_editor_optimized_texture(Platform::UploadContext *c, const Buffer &buffer)
	{
		auto header     = *(TextureBuildHeader *)buffer.storage;

		u16 header_size = sizeof(header);
		byte *src       = buffer.storage + header_size;

		auto *texture   = Platform::init_texture(c, header.width, header.height, Platform::TextureFormat::RGBA8, Platform::TextureUsage::TEXTURE);
		Platform::fill_texture(c, texture, src, buffer.size() - header_size);
		return texture;
	}

	void build_editor_optimized_shader(Buffer *out, const Platform::CompiledShaderSrc &compiled)
	{
		ShaderBuildHeader header{};
		header.version   = 1;
		header.vert_size = compiled.vert_src.size();
		header.frag_size = compiled.frag_src.size();

		u16 header_size  = sizeof(header);
		out->resize(header_size + header.vert_size + header.frag_size);

		out->fill(reinterpret_cast<byte *>(&header), header_size, 0);
		out->fill(compiled.vert_src.storage, header.vert_size, header_size);
		out->fill(compiled.frag_src.storage, header.frag_size, header_size + header.vert_size);
	}

	ErrorOr<Platform::Shader *> load_editor_optimized_shader(Platform::RenderContext *c, const Buffer &buffer)
	{
		auto header     = *(ShaderBuildHeader *)buffer.storage;

		u16 header_size = sizeof(header);

		Platform::CompiledShaderSrc src{};
		src.vert_src = Buffer(buffer.storage + header_size, header.vert_size);
		src.frag_src = Buffer(buffer.storage + header_size + header.vert_size, header.frag_size);

		TRY_UNWRAP(auto reflection, Platform::try_reflect_shader(src));

		return Platform::try_load_shader(c, src, reflection);
	}

	void build_editor_optimized_compute_shader(Buffer *out, const Buffer &compiled)
	{
		ComputeShaderBuildHeader header{};
		header.version  = 1;
		header.size     = compiled.size();

		u16 header_size = sizeof(header);
		out->resize(header_size + header.size);

		out->fill(reinterpret_cast<byte *>(&header), header_size, 0);
		out->fill(compiled.storage, header.size, header_size);
	}

	ErrorOr<Platform::ComputeShader *> load_editor_optimized_compute_shader(Platform::RenderContext *c, const Buffer &buffer)
	{
		auto header     = *(ComputeShaderBuildHeader *)buffer.storage;

		u16 header_size = sizeof(header);

		Buffer src(buffer.storage + header_size, header.size);

		return Platform::try_load_compute_shader(c, src);
	}

	void build_editor_optimized_mesh(Buffer *out, const Platform::ImportedMesh &imported)
	{
		MeshBuildHeader header{};
		header.version      = 1;
		header.vertex_count = imported.vertex_count;
		header.index_count  = imported.index_count;

		u16 header_size     = sizeof(header);
		u64 vertex_size     = sizeof(Platform::Vertex) * imported.vertex_count;
		u64 index_size      = sizeof(u16) * imported.index_count;
		out->resize(header_size + vertex_size + index_size);

		out->fill(reinterpret_cast<byte *>(&header), header_size, 0);
		out->fill(reinterpret_cast<byte *>(imported.vertices), vertex_size, header_size);
		out->fill(reinterpret_cast<byte *>(imported.indices), index_size, header_size + vertex_size);
	}

	ErrorOr<Platform::Mesh *> load_editor_optimized_mesh(Platform::UploadContext *c, const Buffer &buffer)
	{
		auto header        = *(MeshBuildHeader *)buffer.storage;
		u16 header_size    = sizeof(header);
		u64 vertex_size    = header.vertex_count * sizeof(Platform::Vertex);
		u64 index_size     = header.index_count * sizeof(u16);

		auto vertex_buffer = Buffer(buffer.storage + header_size, vertex_size);

		auto index_buffer  = Buffer(buffer.storage + header_size + vertex_size, index_size);

		return Platform::load_mesh_memory(c, vertex_buffer, index_buffer);
	}
} // namespace Vultr
