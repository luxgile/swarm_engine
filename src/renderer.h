#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include "utils.h"
#include "glm/common.hpp"

typedef unsigned int GL_ID;

using namespace glm;
using namespace std;

class Window;
class Shader;
class Mesh;
class Visual;
class Camera;

class RendererBackend {
public:
	vec3 clear_color = vec3(1.0f);

	std::vector<Window*> windows;
	std::vector<Shader*> shaders;
	std::vector<Mesh*> meshes;
	std::vector<Camera*> cameras;
	std::vector<Visual*> visuals;

private:
	void setup_gl();
	void setup_glew();

public:
	RendererBackend();

	Window* create_window(ivec2 size, string title);
	void destroy_window(Window* wnd);

	Shader* create_shader();
	void destroy_shader(Shader* shader);

	Mesh* create_mesh();
	void destroy_mesh(Mesh* mesh);
	
	Camera* create_camera();
	void destroy_camera(Camera* camera);
	Camera* get_current_camera();

	Visual* create_visual();
	void destroy_visual(Visual* mesh);
	void draw_visuals();
};

class Window {
public:
	GLFWwindow* gl_wnd;

public:
	Window(ivec2 size, string title);
	void make_current();
	bool should_close();
	void swap_buffers();
};

class Camera {
	mat4 view;
	mat4 proj;

public:
	int priority = 0;

	void set_view(vec3 pos, vec3 target, vec3 up);
	mat4 get_view_mat() const { return view; } 
	void set_proj(float fov, vec2 screen_size, vec2 near_far_plane);
	mat4 get_proj_mat() const { return proj; } 
};

enum ShaderSrcType {
	Vertex,
	Fragment
};
inline int to_gl_define(ShaderSrcType type);

class Shader {
	GL_ID gl_program;

private:
	unsigned int compile_source(ShaderSrcType type, const char* src);

public:
	void compile_shader(const char* vert, const char* frag);
	void use_shader() const;
	void set_matrix4(const char* uniform, mat4 matrix) const;
};

template <typename T>
class FileImport {
public:
	virtual T* load_file(const char* path) = 0;
};
class ShaderImport : FileImport<Shader> {
public:
	Shader* load_file(const char* path) override;
};

class Mesh {
	GL_ID gl_vertex_array; // Holds vertex structure
	GL_ID gl_vertex_buffer; // Holds vertex data
	GL_ID gl_elements_buffer; // Holds triangle data

public:
	Mesh();
	void set_triangles(vector<unsigned int> indices);
	void set_vertices(vector<vec3> vertices);

	void use_mesh() const;
};

class Visual {
	mat4 xform;
	Shader* shader;
	Mesh* mesh;

public:
	void set_xform(mat4 xform) { this->xform = xform; }
	const mat4* get_xform() { return &xform; }
	void set_mesh(Mesh* mesh) { this->mesh = mesh; }
	const Mesh* get_mesh() { return mesh; }
	void set_shader(Shader* shader) { this->shader = shader; }
	const Shader* get_shader() { return shader; }
};
