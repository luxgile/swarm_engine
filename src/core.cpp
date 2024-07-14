#include "core.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

App* App::singleton = nullptr;

App::App() {
	if (singleton != nullptr) {
		std::cout << "ERROR::MULTIPLE_APPLICATIONS\n" << std::endl;
		return;
	}

	singleton = this;

	render_backend = std::make_unique<RendererBackend>();
	render_backend.get()->setup_internals();
}

void App::app_loop() {
	auto render = get_render_backend();
	auto window = render->get_main_window();
	auto viewport = render->viewports.create();
	window->set_viewport(viewport);
	auto world = render->worlds.create();
	world->vp = viewport;

	auto shader_importer = ShaderImport();
	auto model_importer = ModelImport();
	auto texture_importer = Texture2DImport();
	auto cubemap_importer = CubemapTextureImport();

	auto shader = shader_importer.load_file("E:/dev/Swarm/res/pbr");
	shader->set_sampler_id("albedoMap", SamplerID::Albedo);
	shader->set_sampler_id("mraMap", SamplerID::MRA);
	shader->set_sampler_id("normalMap", SamplerID::Normal);
	shader->set_sampler_id("emissiveMap", SamplerID::Emissive);
	shader->set_sampler_id("shadowMaps", SamplerID::Shadows);
	shader->set_sampler_id("skyboxMap", SamplerID::Skybox);

	auto monkey_model = model_importer.load_file("E:/dev/Swarm/res/monkey.glb");
	auto cube_model = model_importer.load_file("E:/dev/Swarm/res/primitives/cube.glb");

	auto uv_texture = texture_importer.load_file("E:/dev/Swarm/res/uv_texture.png");

	auto skybox = render->visuals.create();
	auto skybox_shader = shader_importer.load_file("E:/dev/Swarm/res/skybox/skybox");
	auto skybox_material = render->materials.create();
	skybox_material->set_shader(skybox_shader);
	CubemapTexture* skybox_cube = cubemap_importer.load_file("E:/dev/Swarm/res/skybox/skybox#.png");
	skybox_cube->set_filter(TextureFilter::Linear);
	skybox_cube->set_wrap(TextureWrap::ClampEdge);
	skybox_material->set_texture(SamplerID::Skybox, skybox_cube);
	skybox->set_material(skybox_material);
	skybox->set_model(cube_model);
	world->env->skybox = skybox;

	auto material = render->materials.create<PbrMaterial>();
	material->set_shader(shader);
	material->set_texture(SamplerID::Albedo, uv_texture);
	material->set_texture(SamplerID::Skybox, skybox_cube);
	world->materials.push_back(material);

	auto monkey_visual = render->visuals.create();
	auto xform = glm::identity<mat4>();
	xform = glm::rotate(xform, glm::radians(180.0f), vec3(0, 1, 0));
	xform = glm::translate(xform, vec3(0, 1, 0));
	monkey_visual->set_xform(xform);
	monkey_visual->set_model(monkey_model);
	monkey_visual->set_material(material);
	world->visuals.push_back(monkey_visual);

	auto floor_visual = render->visuals.create();
	floor_visual->set_model(cube_model);
	floor_visual->set_material(material);
	floor_visual->set_xform(glm::translate(glm::scale(glm::identity<mat4>(), vec3(25.0f, 0.1f, 25.0f)), vec3(0, -10.0f, 0)));
	world->visuals.push_back(floor_visual);

	auto ligth = render->lights.create();
	ligth->type = LightType::Point;
	ligth->position = vec3(3.0, 1.0, -1.0);
	ligth->color = vec3(1.0f, 0.8f, 0.8f);
	ligth->intensity = 3.0f;
	world->lights.push_back(ligth);

	auto ligth2 = render->lights.create();
	ligth2->set_cast_shadows(true);
	ligth2->type = LightType::Point;
	ligth2->position = vec3(-3.0, 1.0, -1.0);
	ligth2->color = vec3(0.3, 0.4, 0.8);
	ligth2->intensity = 7.0f;
	world->lights.push_back(ligth2);

	auto sun = render->lights.create();
	sun->set_cast_shadows(true);
	sun->type = LightType::Directional;
	sun->dir = glm::normalize(vec3(0.3, -0.5, 0.2));
	sun->color = vec3(1.0, 1.0, 1.0);
	sun->intensity = 1.0f;
	world->lights.push_back(sun);

	auto sun2 = render->lights.create();
	sun2->set_cast_shadows(true);
	sun2->type = LightType::Directional;
	sun2->dir = glm::normalize(vec3(-0.3, -0.5, 0.2));
	sun2->color = vec3(1.0, 1.0, 1.0);
	sun2->intensity = 1.0f;
	world->lights.push_back(sun2);


	auto proj = glm::perspectiveFov(90.0f, 1280.0f, 720.0f, 0.1f, 100.0f);
	auto camera = render->cameras.create();
	camera->set_proj(70.0f, vec2(1280.0f, 720.0f), vec2(0.1f, 100.0f));
	camera->set_view(vec3(0, 3, -10), vec3(0, 2, 0), vec3(0, 1, 0));
	world->cameras.push_back(camera);

	world->env->clear_color = vec3(0.2, 0.1, 0.3);

	glfwSwapInterval(60);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

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
		// Most likely flec
		float x = glm::cos(app_time / 5.0f) * 10;
		float z = -glm::sin(app_time / 5.0f) * 10;
		camera->set_view(vec3(x, 2.0f, z), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

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

mat4 Transform::get_matrix() {

	if (!is_cached) {
		cached_matrix = glm::identity<mat4>();
		cached_matrix = glm::scale(cached_matrix, scale);
		cached_matrix *= glm::mat4_cast(rotation);
		cached_matrix = glm::translate(cached_matrix, position);
		is_cached = true;
	}

	return cached_matrix;
}
