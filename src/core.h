#pragma once

#include <memory>
#include "rendering/renderer.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "assets/assets.h"
#include "app_module.h"
#include "world.h"

class AssetBackend;

class App {
private:
	bool app_started = false;
	unsigned int target_fps;

	std::unique_ptr<RendererBackend> render_backend;
	std::unique_ptr<AssetBackend> asset_backend;

	std::unique_ptr<World> main_world;

	std::vector<AplicationModule*> modules;

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

	template<typename T>
	T* add_module() {
		auto mod = new T();
		modules.push_back(static_cast<AplicationModule*>(mod));
		if(app_started) mod->setup();
		return mod;
	}

	void app_loop();
};

//class Transform {
//	bool is_cached;
//	vec3 position;
//	quat rotation;
//	vec3 scale;
//
//	mat4 cached_matrix;
//
//	public:
//	mat4 get_matrix();
//};

