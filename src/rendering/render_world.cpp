#include "render_world.h"
#include "../core.h"

RenderWorld::RenderWorld() {
	env = App::get_render_backend()->enviroments.create();
}

Option<Camera*> RenderWorld::get_active_camera() {
	Option<Camera*> active = None;
	int min_priority = 999999;
	for (auto c : cameras) {
		if (c->priority < min_priority) {
			min_priority = c->priority;
			active = c;
		}
	}
	if (!active) return None;
	return active;
}

bool RenderWorld::is_ready() const {
	//return vp != nullptr && env != nullptr && cameras.size() > 0;
	return true;
}

Viewport::Viewport() {
	fbo = App::get_render_backend()->frame_buffers.create();
	fbo_color = App::get_render_backend()->textures.create();
	fbo_depth_stencil = App::get_render_backend()->textures.create();

	auto size = get_size();
	fbo_color->set_as_rgb8(size.x, size.y, 0);
	fbo_depth_stencil->set_as_depth_stencil(size.x, size.y, 0);

	fbo->set_output_color(fbo_color, 0);
	fbo->set_output_depth_stencil(fbo_depth_stencil);
}

void Viewport::use_viewport() {
	glViewport(0, 0, size.x, size.y);
	fbo->use_framebuffer();
}

void Viewport::set_size(vec2 size) {
	this->size = size;
	fbo_color->set_as_rgb8(size.x, size.y, 0);
	fbo_depth_stencil->set_as_depth_stencil(size.x, size.y, 0);
}
