#include "render_system.h"
#include <vultr.h>
#include <platform/rendering.h>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace Vultr
{
	namespace RenderSystem
	{

		struct MaterialUBO
		{
			Vec4 albedo{};
			f32 metallic;
			f32 ambient_occlusion;
			f32 roughness;
		};

		static Component *component(void *component) { return static_cast<Component *>(component); }
		Component *init()
		{
			auto *c        = v_alloc<Component>();

			auto signature = signature_from_components<Transform, Mesh, Material>();
			register_system(c, signature, entity_created, entity_destroyed);

			c->output_framebuffer = nullptr;
			reinitialize(c);

			//
			//			c->pipeline = Platform::init_pipeline(engine()->context, c->output_framebuffer, info);
			//
			return c;
		}
		void entity_created(void *system, Entity entity) {}
		void entity_destroyed(void *system, Entity entity) {}

		static Mat4 model_matrix(const Transform &transform)
		{
			Mat4 scaling_matrix   = glm::scale(transform.scale);
			Mat4 rotation_matrix  = glm::toMat4(transform.rotation);
			Mat4 translate_matrix = glm::translate(transform.position);
			return translate_matrix * rotation_matrix * scaling_matrix;
		}
		static Mat4 view_matrix(const Transform &transform) { return glm::lookAt(transform.position, transform.position + forward(transform), Vec3(0, 1, 0)); }
		static Mat4 projection_matrix(const Camera &camera, f32 screen_width, f32 screen_height) { return glm::perspective(camera.fov, (f64)screen_width / (f64)screen_height, camera.znear, camera.zfar); }

		static void render(Platform::CameraUBO camera_ubo, Component *system, Platform::CmdBuffer *cmd)
		{
			bool update_light = false;
			for (auto [light, transform_component, directional_light] : get_entities<Transform, DirectionalLight>())
			{
				Platform::DirectionalLightUBO ubo{
					.direction = Vec4(forward(transform_component), 0),
					.ambient   = Vec4(163, 226, 253, 1),
					.diffuse   = Vec4(2000),
					.specular  = 1000,
					.intensity = 2000,
					.exists    = true,
				};
				Platform::update_default_descriptor_set(cmd, &camera_ubo, &ubo);
				update_light = true;
				break;
			}
			ASSERT(update_light, "No directional light found!");

			for (auto [entity, transform, mesh, material] : get_entities<Transform, Mesh, Material>())
			{
				if (!mesh.source.loaded() || !material.source.loaded())
					continue;

				auto model        = model_matrix(transform);
				auto *loaded_mesh = mesh.source.value();
				auto *loaded_mat  = material.source.value();

				if (!system->pipelines.contains(material.source))
				{
					Platform::GraphicsPipelineInfo info{.shader = loaded_mat->source.value()};
					auto *pipeline = Platform::init_pipeline(engine()->context, system->output_framebuffer, info);
					system->pipelines.set(material.source, pipeline);
				}

				auto *pipeline = system->pipelines.get(material.source);

				Platform::PushConstant push_constant{
					.color = Vec4(1),
					.model = model,
				};

				Platform::push_constants(cmd, pipeline, push_constant);
				Platform::bind_material(cmd, pipeline, loaded_mat);
				Platform::draw_mesh(cmd, loaded_mesh);
			}
		}

		void update(const Camera &camera, const Transform &transform, Platform::CmdBuffer *cmd, Component *system)
		{
			Platform::begin_framebuffer(cmd, system->output_framebuffer);

			auto width  = Platform::get_window_width(engine()->window);
			auto height = Platform::get_window_height(engine()->window);
			Platform::CameraUBO camera_ubo{
				.position  = Vec4(transform.position, 0),
				.view      = view_matrix(transform),
				.proj      = projection_matrix(camera, static_cast<f32>(width), static_cast<f32>(height)),
				.view_proj = camera_ubo.proj * camera_ubo.view,
			};
			render(camera_ubo, system, cmd);

			Platform::end_framebuffer(cmd);
		}

		void update(Platform::CmdBuffer *cmd, Component *system)
		{
			Entity camera = 0;
			for (auto [entity, transform_component, camera_component] : get_entities<Transform, Camera>())
			{
				camera = entity;
				break;
			}

			if (camera == 0)
				fprintf(stderr, "No camera found!\n");
			else
				update(get_component<Camera>(camera), get_component<Transform>(camera), cmd, system);
		}

		void update(Component *system)
		{
			if check (Platform::begin_cmd_buffer(engine()->window), auto *cmd, auto _)
			{
				update(cmd, system);
				Platform::end_cmd_buffer(cmd);
			}
			else
			{
				reinitialize(system);
			}
		}

		void reinitialize(Component *c)
		{
			const auto attachment_descriptions = Vector<Platform::AttachmentDescription>({{.format = Platform::TextureFormat::RGBA8}, {.format = Platform::TextureFormat::DEPTH}});
			if (c->output_framebuffer != nullptr)
				Platform::destroy_framebuffer(engine()->context, c->output_framebuffer);
			c->output_framebuffer = Platform::init_framebuffer(engine()->context, attachment_descriptions);
		}

		void destroy(Component *c)
		{
			for (auto &[_, pipeline] : c->pipelines)
			{
				Platform::destroy_pipeline(engine()->context, pipeline);
			}
			Platform::destroy_framebuffer(engine()->context, c->output_framebuffer);
			v_free(c);
		}
	} // namespace RenderSystem
} // namespace Vultr
