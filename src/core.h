#pragma once

#include <memory>
#include "rendering/renderer.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include "assets/assets.h"

class AssetBackend;

class App {
private:
	unsigned int target_fps;

	std::unique_ptr<RendererBackend> render_backend;
	std::unique_ptr<AssetBackend> asset_backend;

	static App* singleton;

private:
	static App* get_singleton() {
		return singleton;
	}

public:
	App();

	void set_target_fps(unsigned int target) { target_fps = target; }

	static RendererBackend* get_render_backend() { return singleton->_get_render_backend(); }
	RendererBackend* _get_render_backend() { return render_backend.get(); }

	static AssetBackend* get_asset_backend() { return singleton->_get_asset_backend(); }
	AssetBackend* _get_asset_backend() { return asset_backend.get(); }

	void app_loop();
};

class Transform {
	bool is_cached;
	vec3 position;
	quat rotation;
	vec3 scale;

	mat4 cached_matrix;

	public:
	mat4 get_matrix();
};

