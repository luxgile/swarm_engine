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
	auto shader = shader_importer.load_file("E:/dev/Swarm/res/unlit");

	vector<vec3> vertices = {
		vec3(0.5f,  0.5f, 0.0f),  // top right
		vec3(0.5f, -0.5f, 0.0f),  // bottom right
		vec3(-0.5f, -0.5f, 0.0f),  // bottom left
		vec3(-0.5f,  0.5f, 0.0f)   // top left
	};
	vector<unsigned int> triangles = {
		0, 1, 3,
		1, 2, 3,
	};
	auto mesh = new Mesh();
	mesh->set_vertices(vertices);
	mesh->set_triangles(triangles);

	auto xform = glm::identity<mat4>();
	xform = glm::translate(xform, vec3(0, 0, 0));

	auto visual = render->create_visual();
	visual->set_xform(xform);
	visual->set_mesh(mesh);
	visual->set_shader(shader);

	auto proj = glm::perspectiveFov(70.0f, 1280.0f, 720.0f, 0.1f, 100.0f);
	auto camera = render->create_camera();
	camera->set_view(vec3(0, 3, -5), vec3(0, 0, 0), vec3(0, 1, 0));
	camera->set_proj(70.0f, vec2(1280.0f, 720.0f), vec2(0.1f, 100.0f));

	render->clear_color = vec3(0.2, 0.1, 0.3);

	while (render->windows.size() > 0) {
		// Remove windows that need to be closed
		for (auto wnd : render->windows) {
			if (wnd->should_close()) {
				render->destroy_window(wnd);
				return;
			}
		}

		// Logic here
		// Most likely flec

		// Render here

		//glEnable(GL_DEPTH_TEST); // enable depth-testing
		//glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

		render->draw_visuals();

		// End frame for all windows
		for (auto wnd : render->windows) {
			wnd->swap_buffers();
			glfwPollEvents();
		}
	}
}

