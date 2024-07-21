#include "core.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "assets/assets.h"
#include "../src_editor/editor_module.h"
#include "rendering/render_plugin.h"
#include "logging.h"

App* App::singleton = nullptr;

App::App() {
	if (singleton != nullptr) {
		Console::log_error("Multiple applications detected, make sure only one has been constructed.");
		return;
	}

	singleton = this;
	target_fps = 60;

	asset_backend = std::make_unique<AssetBackend>();
	asset_backend.get()->set_asset_folder(std::string("E:/dev/Swarm/res/"));
	asset_backend.get()->register_importer<GPUShaderImport>();
	asset_backend.get()->register_importer<GPUModelImport>();
	asset_backend.get()->register_importer<GPUCubemapTextureImport>();
	asset_backend.get()->register_importer<GPUTexture2DImport>();

	render_backend = std::make_unique<RendererBackend>();
	auto rrender = render_backend.get()->setup();
	if (!rrender) Console::log_critical("Render backend failed to initialize:\n{}", rrender.error().error.c_str());

	// Create the obligatory world.
	auto main_world = create_world();
	main_world->add_plugin<RenderPlugin>();

	app_started = true;
}

World* App::create_world() {
	auto world = new World();
	singleton->worlds.push_back(world);
	return world;
}

void App::app_loop() {
	App::add_module<EditorModule>();
	
	auto assets = App::get_asset_backend();

	auto render = get_render_backend();
	auto viewport = render->viewports.create();
	viewport->set_size({1280, 720});
	auto world = get_main_world()->get_ecs()->get<CRenderWorld>()->world;
	world->vp = viewport;

	auto shader = *assets->load_file<GPUShader>("pbr");
	shader->set_sampler_id("albedoMap", SamplerID::Albedo);
	shader->set_sampler_id("mraMap", SamplerID::MRA);
	shader->set_sampler_id("normalMap", SamplerID::Normal);
	shader->set_sampler_id("emissiveMap", SamplerID::Emissive);
	shader->set_sampler_id("shadowMaps", SamplerID::Shadows);
	shader->set_sampler_id("skyboxMap", SamplerID::Skybox);

	auto monkey_model = *assets->load_file<GPUModel>("monkey.glb");
	auto cube_model = *assets->load_file<GPUModel>("primitives/cube.glb");

	auto uv_texture = *assets->load_file<GPUTexture2D>("uv_texture.png");

	auto skybox = render->visuals.create();
	auto skybox_shader = *assets->load_file<GPUShader>("skybox/skybox");
	auto skybox_material = render->materials.create();
	skybox_material->set_shader(skybox_shader);
	auto skybox_cube = *assets->load_file<GPUCubemapTexture>("skybox/skybox#.png");
	skybox_cube->set_filter(TextureFilter::Linear);
	skybox_cube->set_wrap(TextureWrap::ClampEdge);
	skybox_material->set_texture(SamplerID::Skybox, skybox_cube);
	skybox->set_material(skybox_material);
	skybox->set_model(cube_model);
	world->env.value()->skybox = skybox;

	auto material = render->materials.create<GPUPbrMaterial>();
	material->set_shader(shader);
	material->set_texture(SamplerID::Albedo, uv_texture);
	material->set_texture(SamplerID::Skybox, skybox_cube);
	world->materials.push_back(material);

	auto monkey_visual = render->visuals.create();
	auto xform = glm::identity<glm::mat4>();
	xform = glm::rotate(xform, glm::radians(180.0f), glm::vec3(0, 1, 0));
	xform = glm::translate(xform, glm::vec3(0, 1, 0));
	monkey_visual->set_xform(xform);
	monkey_visual->set_model(monkey_model);
	monkey_visual->set_material(material);
	world->visuals.push_back(monkey_visual);

	auto floor_visual = render->visuals.create();
	floor_visual->set_model(cube_model);
	floor_visual->set_material(material);
	floor_visual->set_xform(glm::translate(glm::scale(glm::identity<glm::mat4>(), glm::vec3(25.0f, 0.1f, 25.0f)), glm::vec3(0, -10.0f, 0)));
	world->visuals.push_back(floor_visual);

	auto ligth = render->lights.create();
	ligth->type = LightType::Point;
	ligth->position = glm::vec3(3.0, 1.0, -1.0);
	ligth->color = glm::vec3(1.0f, 0.8f, 0.8f);
	ligth->intensity = 3.0f;
	world->lights.push_back(ligth);

	auto ligth2 = render->lights.create();
	ligth2->set_cast_shadows(true);
	ligth2->type = LightType::Point;
	ligth2->position = glm::vec3(-3.0, 1.0, -1.0);
	ligth2->color = glm::vec3(0.3, 0.4, 0.8);
	ligth2->intensity = 7.0f;
	world->lights.push_back(ligth2);

	auto sun = render->lights.create();
	sun->set_cast_shadows(true);
	sun->type = LightType::Directional;
	sun->dir = glm::normalize(glm::vec3(0.3, -0.5, 0.2));
	sun->color = glm::vec3(1.0, 1.0, 1.0);
	sun->intensity = 1.0f;
	world->lights.push_back(sun);

	auto sun2 = render->lights.create();
	sun2->set_cast_shadows(true);
	sun2->type = LightType::Directional;
	sun2->dir = glm::normalize(glm::vec3(-0.3, -0.5, 0.2));
	sun2->color = glm::vec3(1.0, 1.0, 1.0);
	sun2->intensity = 1.0f;
	world->lights.push_back(sun2);

	auto proj = glm::perspectiveFov(90.0f, 1280.0f, 720.0f, 0.1f, 100.0f);
	auto camera = render->cameras.create();
	camera->set_proj(70.0f, glm::vec2(1280.0f, 720.0f), glm::vec2(0.1f, 100.0f));
	camera->set_view(glm::vec3(0, 3, -10), glm::vec3(0, 2, 0), glm::vec3(0, 1, 0));
	world->cameras.push_back(camera);

	world->env.value()->clear_color = glm::vec3(0.2, 0.1, 0.3);

	if (target_fps != 0) glfwSwapInterval(target_fps);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	render->debug_backend(world);

	int frame = 0;
	float app_time = 0;
	float last_frame_time = glfwGetTime();
	while (render->windows.size() > 0) {
		// Remove windows that need to be closed
		for (auto wnd : render->windows) {
			if (wnd->should_close()) {
				render->destroy_window(wnd);
				return;
			}
		}
		float start_frame_time = glfwGetTime();
		float dt = start_frame_time - last_frame_time;

		// Logic here
		for(auto world : worlds) world->process_frame(0);

		float x = glm::cos(app_time / 5.0f) * 10;
		float z = -glm::sin(app_time / 5.0f) * 10;
		camera->set_view(glm::vec3(x, 2.0f, z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// Render here
		render->render_worlds();

		// End frame for all windows
		for (auto wnd : render->windows) {
			wnd->swap_buffers();
			glfwPollEvents();
		}

		last_frame_time = start_frame_time;
		app_time += dt;
		frame++;
	}
}

//mat4 Transform::get_matrix() {
//
//	if (!is_cached) {
//		cached_matrix = glm::identity<mat4>();
//		cached_matrix = glm::scale(cached_matrix, scale);
//		cached_matrix *= glm::mat4_cast(rotation);
//		cached_matrix = glm::translate(cached_matrix, position);
//		is_cached = true;
//	}
//
//	return cached_matrix;
//}
