#include "render_system.h"
#include <vultr.h>
#include <platform/rendering.h>

namespace Vultr
{
	namespace RenderSystem
	{
		static Component *component(void *component) { return static_cast<Component *>(component); }
		Component *init()
		{
			auto *c               = v_alloc<Component>();

			auto signature        = signature_from_components<Transform, Mesh, Material>();
			c->skybox_mesh        = Platform::init_skybox(engine()->upload_context);

			c->output_framebuffer = nullptr;
			reinitialize(c);

			resource_allocator<Platform::Shader *>()->free_queue_listener = &c->free_queue_listener;

			return c;
		}

		static void render(Platform::CameraUBO camera_ubo, Component *system, Platform::CmdBuffer *cmd)
		{
			bool update_light = false;
			for (auto [light, transform_component, directional_light] : get_entities<Transform, DirectionalLight>())
			{
				Platform::DirectionalLightUBO ubo{
					.direction = Vec4(forward(*transform_component), 0),
					.diffuse   = directional_light->diffuse,
					.specular  = directional_light->specular,
					.intensity = directional_light->intensity,
					.ambient   = directional_light->ambient,
				};
				Platform::update_default_descriptor_set(cmd, &camera_ubo, &ubo);
				update_light = true;
				break;
			}
			if (!update_light)
			{
				return;
			}

			Platform::GraphicsPipelineInfo pipeline_info{};
			for (auto [entity, transform, mesh, material] : get_entities<Transform, Mesh, Material>())
			{
				if (!mesh->source.loaded() || !material->source.loaded())
					continue;

				auto model        = get_world_transform(entity);
				auto *loaded_mesh = mesh->source.value();
				auto *loaded_mat  = material->source.value();

				Platform::PushConstant push_constant{
					.model = model,
				};

				Platform::bind_material(cmd, loaded_mat, pipeline_info, push_constant);
				Platform::draw_mesh(cmd, loaded_mesh);
			}

			for (auto [entity, _, material_component] : get_entities<Camera, Material>())
			{
				if (!material_component->source.loaded())
					continue;

				auto *skybox_mat = material_component->source.value();
				auto *shader     = skybox_mat->source.value();
				pipeline_info    = {
					   .depth_test  = Platform::DepthTest::LEQUAL,
					   .write_depth = false,
                };

				Platform::bind_material(cmd, skybox_mat, pipeline_info);
				Platform::draw_mesh(cmd, system->skybox_mesh);
			}
		}

		void update(const Camera &camera, const Transform &transform, Platform::CmdBuffer *cmd, Component *system)
		{
			if (system->resize_request)
			{
				reinitialize(system, system->resize_request.value().width, system->resize_request.value().height);
				system->resize_request = None;
			}

			Platform::begin_framebuffer(cmd, system->output_framebuffer);

			auto width  = get_width(system->output_framebuffer);
			auto height = get_height(system->output_framebuffer);
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
			Entity camera = INVALID_ENTITY;
			for (auto [entity, transform_component, camera_component] : get_entities<Transform, Camera>())
			{
				camera = entity;
				break;
			}

			if (camera == INVALID_ENTITY)
			{
				fprintf(stderr, "No camera found!\n");
			}
			else
			{
				Transform transform;
				Math::decompose_transform(get_world_transform(camera), &transform.position, &transform.rotation, &transform.scale);
				update(get_component<Camera>(camera), transform, cmd, system);
			}
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

		void request_resize(Component *c, u32 width, u32 height) { c->resize_request = ResizeRequest{.width = width, .height = height}; }

		void reinitialize(Component *c, Option<u32> width, Option<u32> height)
		{
			const auto attachment_descriptions = Vector<Platform::AttachmentDescription>({{.format = Platform::TextureFormat::SRGBA8}, {.format = Platform::TextureFormat::DEPTH}});
			if (c->output_framebuffer != nullptr)
			{
				if (!width)
					width = Platform::get_width(c->output_framebuffer);

				if (!height)
					height = Platform::get_height(c->output_framebuffer);

				Platform::destroy_framebuffer(engine()->context, c->output_framebuffer);
			}
			c->output_framebuffer = Platform::init_framebuffer(engine()->context, attachment_descriptions, width, height);
		}

		void destroy(Component *c)
		{
			Platform::destroy_framebuffer(engine()->context, c->output_framebuffer);
			Platform::destroy_mesh(engine()->context, c->skybox_mesh);
			v_free(c);
		}
	} // namespace RenderSystem
} // namespace Vultr
