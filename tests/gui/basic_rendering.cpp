// #include <gtest/gtest.h>
// #include <gui/core/context.h>
// #include <gui/widgets/button.h>
// #include <gui/widgets/rounded_rect.h>
// #include <gui/widgets/image.h>
// #include <gui/widgets/text.h>
// #include <gui/widgets/center.h>
// #include <gui/widgets/align.h>
// #include <gui/widgets/sized.h>
// #include <gui/widgets/stack.h>
// #include <gui/widgets/row.h>
// #include <gui/widgets/column.h>
// #include <gui/widgets/constrained_box.h>
// #include <gui/widgets/padding.h>
// #include <gui/widgets/text_input.h>
// #include <gui/widgets/container.h>
// #include <gui/materials/default_gui_material.h>
// #include <gui/utils/opengl.h>
// #include <filesystem/importers/texture_importer.h>

// #include <vultr.hpp>
// #include "basic_rendering_test.h"

// using namespace Vultr;

// void basic_rendering_test()
// {
//     auto *vultr = engine_alloc(false);

//     if (vultr == nullptr)
//     {
//         return;
//     }

//     auto resource_directory = Directory("/home/brandon/Dev/Monopoly/res");
//     engine_init_vfs(vultr, &resource_directory);

//     float lastTime = 0;

//     // auto window = IMGUI::new_window(vultr);
//     // auto *c = IMGUI::new_context(vultr, window);
//     // Texture texture;
//     // generate_texture(&texture, GL_TEXTURE_2D);

//     // auto texture_source = TextureSource("/home/brandon/Dev/Monopoly/GameEngine/Vultr/tests/gui/troll.png");
//     // TextureImporter::texture_import(&texture, &texture_source);

//     // change_world(vultr, new_world(vultr->component_registry));

//     // Entity camera = create_entity(get_current_world(vultr));
//     // entity_add_component(vultr, camera, TransformComponent());
//     // entity_add_component(vultr, camera, CameraComponent());

//     // u32 count = 0;
//     // bool toggle_fps = false;

//     // Vec2 size = Vec2(1920, 1080);
//     // Vec2 position = Vec2(0, 0);
//     // auto gl_size = IMGUI::gl_get_size(c, size);
//     // auto gl_position = IMGUI::gl_get_position(c, position, gl_size);

//     // auto screen_size = IMGUI::screen_get_size_from_gl(c, gl_size);
//     // auto screen_position = IMGUI::screen_get_position_from_gl(c, gl_position, gl_size);

//     // std::string value = "";
//     while (!InputSystem::get_key(vultr, Input::KEY_ESCAPE) && !vultr->should_close)
//     {
//         const auto &tick = engine_update_game(vultr, lastTime, false);

//         //     // Begin IMGUI
//         //     IMGUI::begin(c, tick);

//         //     // Mouse pos stuff for testing
//         //     auto mouse_pos = InputSystem::get_mouse_position(vultr);
//         //     mouse_pos.y = 1 - mouse_pos.y;

//         //     // FPS counter
//         //     // if (toggle_fps)
//         //     // {
//         //     IMGUI::text(c, __LINE__, std::to_string(tick.m_delta_time * 1000) + " ms",
//         //                 {
//         //                     .font_color = Color(255),
//         //                     .font_size = 9,
//         //                     .line_spacing = 1,
//         //                     .highlight_color = Color(0, 0, 255, 255),
//         //                 });
//         //     // }

//         //     // Center button
//         //     IMGUI::begin_align(c, __LINE__, IMGUI::Alignment::CENTER());
//         //     {
//         //         // IMGUI::begin_container(c, __LINE__,
//         //         //                        {
//         //         //                            .color = Color(255),
//         //         //                        });
//         //         // {
//         //         // }
//         //         // IMGUI::end_container(c);
//         //         // IMGUI::begin_rounded_rect(c, __LINE__, {.corner_radius = 1});
//         //         // {
//         //         // IMGUI::begin_rounded_rect(c, __LINE__, {.corner_radius = 1});
//         //         // {
//         //         // u8 rounded_rect_count = 50;
//         //         // for (u8 i = 0; i < rounded_rect_count; i++)
//         //         // {
//         //         //     IMGUI::begin_rounded_rect(c, IMGUI::ui_id("RoundedRect") + i, {.corner_radius = 1});
//         //         // }
//         //         IMGUI::begin_sized(c, __LINE__, Vec2(2000));
//         //         {
//         //             //     IMGUI::begin_align(c, __LINE__, {.value = Vec2(1, 1)});
//         //             //     {
//         //             //         IMGUI::begin_sized(c, __LINE__, Vec2(500));
//         //             //         {
//         //             //             IMGUI::begin_rounded_rect(c, __LINE__, {.corner_radius = 0.1});
//         //             //             {
//         //             //                 IMGUI::container(c, __LINE__, {.color = Color(255, 0, 0, 255)});
//         //             //             }
//         //             //             IMGUI::end_rounded_rect(c);
//         //             //         }
//         //             //         IMGUI::end_sized(c);
//         //             //     }
//         //             //     IMGUI::end_align(c);

//         //             // IMGUI::begin_container(c, __LINE__, {.color = Color(255)});
//         //             // {

//         //             IMGUI::begin_column(c, __LINE__, {});
//         //             {
//         //                 Color colors[5] = {Color(255, 0, 0), Color(0, 255, 0), Color(0, 0, 255), Color(255), Color(255, 0, 0)};
//         //                 for (u32 i = 0; i < 3; i++)
//         //                 {
//         //                     IMGUI::begin_column_element(c, i);
//         //                     {
//         //                         IMGUI::begin_sized(c, IMGUI::ui_id("ColumnElementSized") + i, Vec2(300 + i * 100));
//         //                         {
//         //                             IMGUI::begin_rounded_rect(c, IMGUI::ui_id("ROUNDED_RECT") + i, {.corner_radius = 0.1});
//         //                             {
//         //                                 IMGUI::container(c, IMGUI::ui_id("ColumnElementContainer") + i, {.color = colors[i]});
//         //                             }
//         //                             IMGUI::end_rounded_rect(c);
//         //                         }
//         //                         IMGUI::end_sized(c);
//         //                     }
//         //                     IMGUI::end_column_element(c);
//         //                 }
//         //                 IMGUI::begin_column_element(c);
//         //                 {
//         //                     if (IMGUI::text_button(c, __LINE__, std::to_string(count),
//         //                                            {
//         //                                                .background_color = Color(255),
//         //                                                .text_style =
//         //                                                    {
//         //                                                        .font_color = Color(0, 255),
//         //                                                    },
//         //                                            }))
//         //                     {
//         //                         count++;
//         //                         toggle_fps = !toggle_fps;
//         //                     }
//         //                 }
//         //                 IMGUI::end_column_element(c);
//         //             }
//         //             IMGUI::end_column(c);
//         //             // }
//         //             // IMGUI::end_container(c);
//         //         }
//         //         IMGUI::end_sized(c);
//         //         // for (u8 i = 0; i < rounded_rect_count; i++)
//         //         // {
//         //         //     IMGUI::end_rounded_rect(c);
//         //         // }
//         //         // }
//         //         // IMGUI::end_rounded_rect(c);
//         //         // }
//         //         // IMGUI::end_rounded_rect(c);
//         //     }
//         //     IMGUI::end_align(c);

//         //     IMGUI::end(c);

//         glfwSwapBuffers(vultr->window);
//         glfwPollEvents();
//     }

//     engine_free(vultr);
// }
