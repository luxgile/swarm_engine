#pragma once
#include "renderer.h"
#include <glm/glm.hpp>
#include "glm/common.hpp"
#include "boost/signals2.hpp"
#include <imgui.h>
#include "../venum.h"

using namespace glm;
using namespace std;

class Camera;
struct Light;
class GPUMaterial;
class GPUVisual;
class GPUFrameBuffer;
class GPUTexture2D;

class Viewport {
	vec2 size;
	GPUTexture2D* fbo_color;
	GPUTexture2D* fbo_depth_stencil;

public:
	Viewport();
	GPUFrameBuffer* fbo;

	void use_viewport();

	void set_size(vec2 size);
	vec2 get_size() const { return size; }
	
	Option<GPUTexture2D*> get_color_ouput() const { return fbo_color; }
	Option<GPUTexture2D*> get_depth_ouput() const { return fbo_depth_stencil; }
};

class RenderEnviroment {
public:
	vec3 clear_color;
	vec3 ambient_color = vec3(0.3f, 0.3f, 0.1f);
	float ambient_intensity = 0.3f;

	GPUVisual* skybox;
};

class RenderWorld {

public:
	RenderWorld();

	Option<Viewport*> vp;
	Option<RenderEnviroment*> env;
	Option<ImDrawData*> imgui_draw_cmd;

	boost::signals2::signal<void()> on_pre_render;
	boost::signals2::signal<void()> on_ui_pass;
	boost::signals2::signal<void()> on_post_render;

	vector<Camera*> cameras;
	vector<Light*> lights;
	vector<GPUMaterial*> materials;
	vector<GPUVisual*> visuals;

	Option<Camera*> get_active_camera();

	bool is_ready() const;
};

