#pragma once
#include <map>
#include <render/types/mesh.h>
#include <string>
#include <filesystem/file.h>

namespace Vultr::MeshImporter
{
    bool mesh_import(Mesh *mesh, const ModelSource *source);
    bool mesh_import_file(Mesh *mesh, const ModelSource *source);
    bool mesh_import_memory(Mesh *mesh, const unsigned char *data, u64 size);
    void mesh_load_gpu(Mesh *mesh);
    void init_quad(Mesh *mesh);
    void init_skybox(Mesh *mesh);

} // namespace Vultr::MeshImporter
