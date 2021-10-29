#include <render/types/mesh.h>
#include <glad/glad.h>

namespace Vultr
{

    bool is_valid_mesh(const Mesh *mesh)
    {
        return mesh->vertices != nullptr;
    }

    void new_mesh(Mesh *m, Vec3 positions[], Vec2 uvs[], Vec3 normals[], size_t vertex_count, u16 indices[], size_t index_count, bool load_gpu)
    {
        auto *vertices = static_cast<Vertex *>(malloc(sizeof(Vertex) * vertex_count));

        for (size_t i = 0; i < vertex_count; i++)
        {
            vertices[i] = Vertex(positions[i], normals[i], uvs[i]);
        }

        free(positions);
        free(uvs);
        free(normals);

        m->vertices = vertices;
        m->vertex_count = vertex_count;
        m->indices = indices;
        m->index_count = index_count;

        if (load_gpu)
        {
            mesh_init_gpu(m);
        }
    }

    void new_mesh(Mesh *m, Vertex vertices[], size_t vertex_count, u16 indices[], size_t index_count, bool load_gpu)
    {
        m->vertices = vertices;
        m->vertex_count = vertex_count;
        m->indices = indices;
        m->index_count = index_count;
        if (load_gpu)
        {
            mesh_init_gpu(m);
        }
    }

    void mesh_init_gpu(Mesh *m)
    {
        auto vao = new_vertex_array();
        bind_vertex_array(vao);

        auto vbo = new_vertex_buffer(m->vertices, m->vertex_count);
        bind_vertex_buffer(vbo);

        setup_vertex_array<Vertex>();

        auto ibo = new_index_buffer(m->indices, m->index_count);

        m->vao = vao;
        m->ibo = ibo;
        m->vbo = vbo;
    }

    void delete_mesh(Mesh *mesh)
    {
        free(mesh->vertices);
        free(mesh->indices);
        mesh->vertices = nullptr;
        mesh->vertex_count = 0;
        mesh->indices = nullptr;
        mesh->index_count = 0;

        delete_vertex_array(mesh->vao);
        delete_index_buffer(mesh->ibo);
        delete_vertex_buffer(mesh->vbo);
    }

    void draw_mesh(const Mesh *mesh)
    {
        bind_vertex_array(mesh->vao);
        bind_index_buffer(mesh->ibo);

        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_SHORT, (void *)0);
    }
} // namespace Vultr
