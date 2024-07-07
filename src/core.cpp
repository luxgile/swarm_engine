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
}

void App::app_loop() {
	auto render = get_render_backend();

	auto shader_importer = ShaderImport();
	auto shader = shader_importer.load_file("E:/dev/Swarm/res/pbr");
	auto model_importer = ModelImport();
	auto monkey_model = model_importer.load_file("E:/dev/Swarm/res/monkey.glb");
	auto floor_model = model_importer.load_file("E:/dev/Swarm/res/cube.glb");

	auto monkey_visual = render->create_visual();
	monkey_visual->set_model(monkey_model);
	monkey_visual->set_shader(shader);

	auto floor_visual = render->create_visual();
	floor_visual->set_model(floor_model);
	floor_visual->set_shader(shader);
	floor_visual->set_xform(glm::translate(glm::scale(glm::identity<mat4>(), vec3(25.0f, 0.1f, 25.0f)), vec3(0, -10.0f, 0)));

	auto ligth = render->create_point_light();
	ligth->position = vec3(3.0, 1.0, -1.0);
	ligth->color = vec3(1.0f, 0.8f, 0.8f);
	ligth->intensity = 30.0f;

	auto ligth2 = render->create_point_light();
	ligth2->position = vec3(-3.0, 1.0, -1.0);
	ligth2->color = vec3(0.3, 0.4, 0.8);
	ligth2->intensity = 70.0f;

	auto proj = glm::perspectiveFov(70.0f, 1280.0f, 720.0f, 0.1f, 100.0f);
	auto camera = render->create_camera();
	camera->set_proj(70.0f, vec2(1280.0f, 720.0f), vec2(0.1f, 100.0f));
	camera->set_view(vec3(0, 3, -7), vec3(0, 0, 0), vec3(0, 1, 0));
	render->clear_color = vec3(0.2, 0.1, 0.3);

	glfwSwapInterval(60);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

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
		float y = 1 + glm::sin(app_time / 0.5f) * 0.5f;
		auto xform = glm::identity<mat4>();
		xform = glm::rotate(xform, glm::radians(180.0f), vec3(0, 1, 0));
		xform = glm::translate(xform, vec3(0, y, 0));
		monkey_visual->set_xform(xform);

		// Render here
		render->draw_visuals();

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

