#include "core.h"
#include <iostream>

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

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader->use_shader();
		mesh->use_mesh();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// End frame for all windows
		for (auto wnd : render->windows) {
			wnd->swap_buffers();
			glfwPollEvents();
		}
	}
}

