// TODO: Reimplement with new engine
// #include <gtest/gtest.h>
// #define private public
// #define protected public

// #include <engine.hpp>
// #include <filesystem/virtual_filesystem.h>

// using namespace Vultr;

// TEST(ResourceManager, Init)
// {
//     auto *e = new Engine();
//     e->resource_manager = new ResourceManager();

//     // engine_init_resource_threads(e);

//     auto resource_directory = Directory("./.test_resources/");
//     ASSERT_TRUE(direxists(&resource_directory));

//     engine_init_vfs(e, &resource_directory);

//     auto texture = RESOURCE(Texture, "test.jpg", e);
//     // auto m1 = RESOURCE(Mesh, "mesh.obj", e);

//     // auto shader = RESOURCE(Shader, "shader.glsl", e);

//     // usleep(1000);

//     // auto dup_m = RESOURCE(Mesh, "mesh.obj", e);
//     // auto m2 = RESOURCE(Mesh, "mesh2.obj", e);

//     auto item = *e->resource_manager->load_queue.front();
//     e->resource_manager->load_queue.pop_wait();

//     printf("Loading resource queue item %u!\n", item.file);

//     e->resource_manager->load_asset(e->vfs, &item);

//     // USleeps are necessary to make sure that we don't remove the files too early
//     // usleep(1000);
// }
