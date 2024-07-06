#include "core.h"

App::App() {
	render_backend = std::make_unique<RendererBackend>();
}
