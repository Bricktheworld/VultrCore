#include "../rendering.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Vultr
{
	namespace Platform
	{
		static ImportedMesh mesh_import_scene(const aiScene *scene)
		{
			const aiMesh *imported_mesh = scene->mMeshes[0];
			const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

			auto *vertices = v_alloc<Vertex>(imported_mesh->mNumVertices);
			for (size_t i = 0; i < imported_mesh->mNumVertices; i++)
			{
				const aiVector3D *pPos       = &(imported_mesh->mVertices[i]);
				const aiVector3D *pNormal    = &(imported_mesh->mNormals[i]);
				const aiVector3D *pTexCoord  = imported_mesh->HasTextureCoords(0) ? &(imported_mesh->mTextureCoords[0][i]) : &Zero3D;
				const aiVector3D *pTangent   = &(imported_mesh->mTangents[i]);
				const aiVector3D *pBitangent = &(imported_mesh->mBitangents[i]);

				vertices[i]                  = {
									 .position  = Vec3(pPos->x, pPos->y, pPos->z),
									 .normal    = Vec3(pNormal->x, pNormal->y, pNormal->z),
									 .uv        = Vec2(pTexCoord->x, pTexCoord->y),
									 .tangent   = Vec3(pTangent->x, pTangent->y, pTangent->z),
									 .bitangent = Vec3(pBitangent->x, pBitangent->y, pBitangent->z),
                };
			}

			u16 *indices = v_alloc<u16>(imported_mesh->mNumFaces * 3);
			size_t index = 0;
			for (size_t i = 0; i < imported_mesh->mNumFaces; i++)
			{
				const aiFace &face = imported_mesh->mFaces[i];
				ASSERT(face.mNumIndices == 3, "Invalid number of indices!");
				indices[index]     = face.mIndices[0];
				indices[index + 1] = face.mIndices[1];
				indices[index + 2] = face.mIndices[2];
				index += 3;
			}
			//			auto *mesh = init_mesh(c, vertices, imported_mesh->mNumVertices, indices, imported_mesh->mNumFaces * 3);
			return {.vertices = vertices, .indices = indices};
		}

		ErrorOr<ImportedMesh> import_mesh_file(const Path &path)
		{
			Assimp::Importer importer;

			const aiScene *scene = importer.ReadFile(path.c_str(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
			if (!scene)
			{
				return Error("Failed to import scene " + path.string());
			}

			auto mesh = mesh_import_scene(scene);
			importer.FreeScene();

			return mesh;
		}
		ErrorOr<ImportedMesh> import_mesh_memory(const Buffer &buffer)
		{
			Assimp::Importer importer;
			const aiScene *scene = importer.ReadFileFromMemory(buffer.storage, buffer.size(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
			if (!scene)
			{
				return Error("Failed to import scene!");
			}

			auto mesh = mesh_import_scene(scene);
			importer.FreeScene();

			return mesh;
		}
		void free_imported_mesh(ImportedMesh *mesh)
		{
			v_free(mesh->vertices);
			v_free(mesh->indices);
		}
	} // namespace Platform
} // namespace Vultr
