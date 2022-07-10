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
			c->depth_prepass_fbo  = nullptr;

			c->compute_shader     = Resource<Platform::ComputeShader *>(Platform::parse_uuid("b741af72-5471-4d04-855b-7dc2d9de9f27"));

			reinitialize(c);

			//
			resource_allocator<Platform::Shader *>()->free_queue_listener = &c->free_queue_listener;

			return c;
		}

		static void render(Component *system, Platform::CmdBuffer *cmd)
		{
			Platform::begin_framebuffer(cmd, system->output_framebuffer);

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

				auto *skybox_mat          = material_component->source.value();
				auto *shader              = skybox_mat->source.value();

				pipeline_info.depth_test  = Platform::DepthTest::LEQUAL;
				pipeline_info.write_depth = false;

				Platform::bind_material(cmd, skybox_mat, pipeline_info);
				Platform::draw_mesh(cmd, system->skybox_mesh);
			}

			Platform::end_framebuffer(cmd);
		}

		static bool update_default_descriptors(const Camera &camera, const Transform &transform, Platform::CmdBuffer *cmd, Component *system)
		{
			auto width  = get_width(system->output_framebuffer);
			auto height = get_height(system->output_framebuffer);

			Platform::CameraUBO camera_ubo{
				.position  = Vec4(transform.position, 0),
				.view      = view_matrix(transform),
				.proj      = projection_matrix(camera, static_cast<f32>(width), static_cast<f32>(height)),
				.view_proj = camera_ubo.proj * camera_ubo.view,
			};

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
				return false;
			}

			return true;
		}

		static void depth_prepass(Component *system, Platform::CmdBuffer *cmd)
		{
			Platform::begin_framebuffer(cmd, system->depth_prepass_fbo);
			Platform::GraphicsPipelineInfo info{
				.shader_usage = Platform::ShaderUsage::VERT_ONLY,
				.depth_test   = Platform::DepthTest::LEQUAL,
				.write_depth  = true,
			};

			for (auto [entity, transform, mesh, material] : get_entities<Transform, Mesh, Material>())
			{
				if (!mesh->source.loaded() || !material->source.loaded())
					continue;

				auto model        = get_world_transform(entity);
				auto *loaded_mesh = mesh->source.value();
				auto *loaded_mat  = material->source.value();

				Platform::bind_material(cmd, loaded_mat, info, Platform::PushConstant{model});
				Platform::draw_mesh(cmd, loaded_mesh);
			}

			Platform::end_framebuffer(cmd);
		}

		void update(const Camera &camera, const Transform &transform, Platform::CmdBuffer *cmd, Component *system)
		{
			if (system->resize_request)
			{
				reinitialize(system, system->resize_request.value().width, system->resize_request.value().height);
				system->resize_request = None;
			}

			if (!system->compute_shader.loaded() || !update_default_descriptors(camera, transform, cmd, system))
			{
				Platform::begin_framebuffer(cmd, system->output_framebuffer);
				Platform::end_framebuffer(cmd);
				return;
			}

			depth_prepass(system, cmd);
			auto *input  = Platform::get_attachment_texture(system->depth_prepass_fbo, 0);
			auto *output = Platform::get_attachment_texture(system->output_framebuffer, 0);

			auto *cs     = system->compute_shader.value();
			Platform::update_compute_shader(cs, input, output);
			Platform::dispatch_compute(cmd, cs);

			Platform::begin_framebuffer(cmd, system->output_framebuffer);
			Platform::end_framebuffer(cmd);

			// render(system, cmd);

			// Platform::begin_framebuffer(cmd, system->output_framebuffer);

			// render(camera_ubo, system, cmd);

			// Platform::end_framebuffer(cmd);
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
			if (c->output_framebuffer != nullptr)
			{
				if (!width)
					width = Platform::get_width(c->output_framebuffer);

				if (!height)
					height = Platform::get_height(c->output_framebuffer);

				Platform::destroy_framebuffer(engine()->context, c->output_framebuffer);
			}

			if (c->depth_prepass_fbo != nullptr)
				Platform::destroy_framebuffer(engine()->context, c->depth_prepass_fbo);

			Vector<Platform::AttachmentDescription> output_attachment_descriptions({
				{.format = Platform::TextureFormat::SRGBA8},
				{.format = Platform::TextureFormat::DEPTH},
			});
			c->output_framebuffer = Platform::init_framebuffer(engine()->context, output_attachment_descriptions, width, height);

			Vector<Platform::AttachmentDescription> depth_prepass_attachments({
				{.format = Platform::TextureFormat::DEPTH},
			});
			c->depth_prepass_fbo = Platform::init_framebuffer(engine()->context, depth_prepass_attachments, width, height);
		}

		void free_resources(Component *c) { c->compute_shader = {}; }

		void destroy(Component *c)
		{
			if (c == nullptr)
				return;

			if (c->depth_prepass_fbo != nullptr)
				Platform::destroy_framebuffer(engine()->context, c->depth_prepass_fbo);

			if (c->output_framebuffer != nullptr)
				Platform::destroy_framebuffer(engine()->context, c->output_framebuffer);

			if (c->skybox_mesh != nullptr)
				Platform::destroy_mesh(engine()->context, c->skybox_mesh);

			v_free(c);
		}
	} // namespace RenderSystem
} // namespace Vultr
