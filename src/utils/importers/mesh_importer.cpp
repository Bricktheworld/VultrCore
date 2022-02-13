// TODO: Reimplement using VTL
#include <filesystem/resource_manager.h>
#include <filesystem/importers/mesh_importer.h>

namespace Vultr
{
	template <>
	bool load_resource<Mesh>(const VirtualFilesystem *vfs, VFileHandle file, Mesh *resource, ResourceQueueItem *item)
	{
		// assert(vfs_file_exists(vfs, file) && "Cannot load model, file does not exist!");
		// const char *path = vfs->file_table_path.at(file).path;
		// printf("Loading model %s\n", path);

		// VFileStream *stream = vfs_open(vfs, file, "rb");

		// u64 size = 0;
		// auto *buf = vfs_read_full(vfs, &size, stream);
		// vfs_close(stream);

		// if (buf == nullptr)
		// {
		//     fprintf(stderr, "Failed to load model %s! Something went wrong opening the file...\n", path);
		//     return false;
		// }

		// bool res = MeshImporter::mesh_import_memory(resource, buf, size);
		// vfs_free_buf(buf);

		// if (!res)
		// {
		//     fprintf(stderr, "Failed to load model %s! Something went wrong loading into memory...\n", path);
		//     return false;
		// }

		// item->type = ResourceType::MESH;
		// item->file = file;

		// return true;
	}

	template <>
	bool finalize_resource<Mesh>(VFileHandle file, Mesh *data, void *buffer)
	{
		printf("Finalizing mesh on main thread!\n");

		MeshImporter::mesh_load_gpu(data);

		return true;
	}

	template <>
	void free_resource<Mesh>(Mesh *resource)
	{
		delete_mesh(resource);
	}

	namespace MeshImporter
	{
		static void mesh_import_scene(const aiScene *scene, Mesh *mesh)
		{
			// TODO(Brandon): Reimplement.
			// new_mesh(mesh, vertices, imported_mesh->mNumVertices, indices, imported_mesh->mNumFaces * 3, false);
		}

		bool mesh_import_file(Mesh *mesh, const ModelSource *source) {}

		bool mesh_import_memory(Mesh *mesh, const unsigned char *data, u64 size) {}

		void mesh_load_gpu(Mesh *mesh) { mesh_init_gpu(mesh); }

		bool import_mesh(Mesh *mesh, const ModelSource *source)
		{
			if (!mesh_import_file(mesh, source))
				return false;

			mesh_load_gpu(mesh);

			return true;
		}

		void init_quad(Mesh *mesh)
		{
			// TODO: Add bitangent calculations
			Vertex vertices[] = {
				Vertex(Vec3(-1, -1, 0), Vec3(0), Vec2(0, 0)), // bottom left
				Vertex(Vec3(-1, 1, 0), Vec3(0), Vec2(0, 1)),  // top left
				Vertex(Vec3(1, -1, 0), Vec3(0), Vec2(1, 0)),  // bottom right
				Vertex(Vec3(1, 1, 0), Vec3(0), Vec2(1, 1)),   // top right
			};
			u16 indices[] = {
				1, 2, 3, 1, 0, 2,
			};
			new_mesh(mesh, vertices, 4, indices, 6, true);
		}

		void init_skybox(Mesh *mesh)
		{
			Vertex vertices[] = {
				Vertex(Vec3(-1, -1, -1)), // 0
				Vertex(Vec3(1, -1, -1)),  // 1
				Vertex(Vec3(1, 1, -1)),   // 2
				Vertex(Vec3(1, 1, 1)),    // 3
				Vertex(Vec3(-1, 1, -1)),  // 4
				Vertex(Vec3(-1, 1, 1)),   // 5
				Vertex(Vec3(-1, -1, 1)),  // 6
				Vertex(Vec3(1, -1, 1)),   // 7
				Vertex(Vec3(1, 1, -1)),   // 8
			};
			u16 indices[] = {// Front
							 1, 8, 4, 4, 0, 1,
							 // Left
							 6, 0, 4, 4, 5, 6,
							 // Right
							 1, 7, 3, 3, 8, 1,
							 // Back
							 6, 5, 3, 3, 7, 6,
							 // Top
							 4, 8, 3, 3, 5, 4,
							 // Bottom
							 0, 6, 1, 1, 6, 7};

			new_mesh(mesh, vertices, 9, indices, 36, true);
		}
	} // namespace MeshImporter

} // namespace Vultr
