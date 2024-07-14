#include "render_world.h"
#include "../core.h"

RenderWorld::RenderWorld() {
	env = App::get_render_backend()->enviroments.create();
}

Camera* RenderWorld::get_active_camera() {
	Camera* active = nullptr;
	int min_priority = 999999;
	for (auto c : cameras) {
		if (c->priority < min_priority) {
			min_priority = c->priority;
			active = c;
		}
	}
	return active;
}

Viewport::Viewport() {
	
}
