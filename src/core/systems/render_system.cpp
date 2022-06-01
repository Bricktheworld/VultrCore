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
			while (!system->free_queue_listener.empty())
			{
				auto shader = system->free_queue_listener.pop();
				if (system->pipelines.contains(shader))
				{
					Platform::destroy_pipeline(engine()->context, system->pipelines.get(shader));
					system->pipelines.remove(shader);
				}
				if (system->skybox_pipeline.has_value() && system->skybox_pipeline.value().get<0>() == shader)
				{
					auto [_, pipeline] = system->skybox_pipeline.value();
					Platform::destroy_pipeline(engine()->context, pipeline);
					system->skybox_pipeline = None;
				}
			}
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

			for (auto [entity, transform, mesh, material] : get_entities<Transform, Mesh, Material>())
			{
				if (!mesh->source.loaded() || !material->source.loaded())
					continue;

				auto model        = get_world_transform(entity);
				auto *loaded_mesh = mesh->source.value();
				auto *loaded_mat  = material->source.value();

				if (!system->pipelines.contains(loaded_mat->source.value()))
				{
					Platform::GraphicsPipelineInfo info{.shader = loaded_mat->source.value()};
					auto *pipeline = Platform::init_pipeline(engine()->context, system->output_framebuffer, info);
					system->pipelines.set(static_cast<void *>(loaded_mat->source.value()), pipeline);
				}

				auto *pipeline = system->pipelines.get(loaded_mat->source.value());

				Platform::PushConstant push_constant{
					.model = model,
				};

				Platform::push_constants(cmd, pipeline, push_constant);
				Platform::bind_material(cmd, pipeline, loaded_mat);
				Platform::draw_mesh(cmd, loaded_mesh);
			}

			for (auto [entity, _, material_component] : get_entities<Camera, Material>())
			{
				if (!material_component->source.loaded())
					continue;

				auto *skybox_mat = material_component->source.value();
				auto *shader     = skybox_mat->source.value();
				Platform::GraphicsPipeline *pipeline;
				if (system->skybox_pipeline.has_value())
				{
					if (system->skybox_pipeline.value().get<0>() != shader)
						continue;
					pipeline = system->skybox_pipeline.value().get<1>();
				}
				else
				{
					Platform::GraphicsPipelineInfo info{
						.shader      = shader,
						.depth_test  = Platform::DepthTest::LEQUAL,
						.write_depth = false,
					};

					pipeline                = Platform::init_pipeline(engine()->context, system->output_framebuffer, info);
					system->skybox_pipeline = Tuple<void *, Platform::GraphicsPipeline *>({shader, pipeline});
				}

				Platform::bind_material(cmd, pipeline, skybox_mat);
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
			for (auto &[_, pipeline] : c->pipelines)
			{
				Platform::destroy_pipeline(engine()->context, pipeline);
			}

			if (c->skybox_pipeline)
			{
				Platform::destroy_pipeline(engine()->context, c->skybox_pipeline.value().get<1>());
			}

			Platform::destroy_framebuffer(engine()->context, c->output_framebuffer);
			Platform::destroy_mesh(engine()->context, c->skybox_mesh);
			v_free(c);
		}
	} // namespace RenderSystem
} // namespace Vultr
