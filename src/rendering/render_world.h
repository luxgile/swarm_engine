#pragma once
#include "renderer.h"
#include <glm/glm.hpp>
#include "glm/common.hpp"

using namespace glm;
using namespace std;

class Camera;
struct Light;
class Material;
class Visual;

class Viewport {
	vec2 size;

public:
	Viewport();
	void set_size(vec2 size) { this->size = size; }
};

class RenderEnviroment {
public:
	vec3 clear_color;
	vec3 ambient_color = vec3(0.3f, 0.3f, 0.1f);
	float ambient_intensity = 0.3f;

	Visual* skybox;

};

class RenderWorld {

public:
	RenderWorld();

	Viewport* vp;
	RenderEnviroment* env;

	vector<Camera*> cameras;
	vector<Light*> lights;
	vector<Material*> materials;
	vector<Visual*> visuals;

	Camera* get_active_camera();
};

