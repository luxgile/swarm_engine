#pragma once
#include <memory>
#include "renderer.h"

class App {
	std::unique_ptr<RendererBackend> render_backend;

	App();
};
