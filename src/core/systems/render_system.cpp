#include "render_system.h"
#include "resource_system.h"
#include <vultr.h>
#include <platform/rendering.h>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace Vultr
{
	namespace RenderSystem
	{
		struct CameraUBO
		{
			Vec4 position{};
			Mat4 view{};
			Mat4 proj{};
			Mat4 view_proj{};
		};

		static Component *component(void *component) { return static_cast<Component *>(component); }
		Component *init()
		{
			auto *c        = v_alloc<Component>();

			auto signature = signature_from_components<Transform, Mesh>();
			register_system(c, signature, entity_created, entity_destroyed);

			c->camera_layout = Platform::init_descriptor_layout<CameraUBO>(engine()->context, 1);
			Platform::bind_descriptor_layout(engine()->context, c->camera_layout);

			return c;
		}
		void entity_created(void *system, Entity entity) {}
		void entity_destroyed(void *system, Entity entity) {}

		static Mat4 model_matrix(Transform *transform)
		{
			Mat4 scaling_matrix   = glm::scale(transform->scale);
			Mat4 rotation_matrix  = glm::toMat4(transform->rotation);
			Mat4 translate_matrix = glm::translate(transform->position);
			return translate_matrix * rotation_matrix * scaling_matrix;
		}
		static Mat4 view_matrix(Transform *transform) { return glm::lookAt(transform->position, transform->position + forward(transform), Vec3(0, 1, 0)); }
		static Mat4 projection_matrix(Camera *camera, f32 screen_width, f32 screen_height) { return glm::perspective(camera->fov, screen_width / screen_height, camera->znear, camera->zfar); }

		static void render(Component *system, Platform::CmdBuffer *cmd, ResourceSystem::Component *resource_system)
		{
			Entity camera = 0;
			CameraUBO camera_ubo{};
			for (auto [entity, transform_component, camera_component] : get_entities<Transform, Camera>())
			{
				camera               = entity;
				camera_ubo.position  = Vec4(transform_component.position, 0);
				camera_ubo.view      = view_matrix(&transform_component);
				auto width           = Platform::get_window_width(engine()->window);
				auto height          = Platform::get_window_height(engine()->window);
				camera_ubo.proj      = projection_matrix(&camera_component, static_cast<f32>(width), static_cast<f32>(height));
				camera_ubo.view_proj = camera_ubo.proj * camera_ubo.view;
				break;
			}

			if (camera == 0)
			{
				fprintf(stderr, "No camera found!\n");
				return;
			}

			Platform::update_descriptor_set(cmd, system->camera_layout, &camera_ubo, 0);
			Platform::flush_descriptor_set_changes(cmd);

			Platform::bind_pipeline(cmd, resource_system->pipeline);
			Platform::bind_descriptor_set(cmd, resource_system->pipeline, system->camera_layout, 0, 0);
			for (auto [entity, transform, mesh] : get_entities<Transform, Mesh>())
			{
				auto hash         = Traits<Path>::hash(mesh.source.value_or({}));
				auto *loaded_mesh = resource_system->loaded_meshes.get(hash);

				auto model        = model_matrix(&transform);

				Platform::PushConstant push_constant{
					.color = Vec4(1),
					.model = model,
				};

				Platform::push_constants(cmd, resource_system->pipeline, push_constant);
				Platform::draw_mesh(cmd, loaded_mesh);
			}
		}

		void update(Component *system, ResourceSystem::Component *resource_system)
		{
			if check (Platform::begin_cmd_buffer(engine()->window), auto *cmd, auto _)
			{
				render(system, cmd, resource_system);
				Platform::end_cmd_buffer(cmd);
			}
			else
			{
			}
		}

		void destroy(Component *c)
		{
			Platform::destroy_descriptor_layout(engine()->context, c->camera_layout);
			v_free(c);
		}
	} // namespace RenderSystem
} // namespace Vultr
