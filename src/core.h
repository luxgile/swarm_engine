#ifndef CORE_HEADER
#define CORE_HEADER

#include <memory>
#include "renderer.h"


class App {
private:
	std::unique_ptr<RendererBackend> render_backend;

	static App* singleton;

private:
	static App* get_singleton() {
		return singleton;
	}

public:
	App();

	static RendererBackend* get_render_backend() { return singleton->_get_render_backend(); }
	RendererBackend* _get_render_backend() { return render_backend.get(); }

	void app_loop();
};

#endif // !CORE_HEADER
