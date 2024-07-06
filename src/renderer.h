#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include "utils.h"

typedef unsigned int GL_ID;

using namespace glm;
using namespace std;

class Window;
class Shader;
class Mesh;

class RendererBackend {
public:
	std::vector<Window*> windows;
	std::vector<Shader*> shaders;
	std::vector<Mesh*> meshes;

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
	void use_shader();
	void set_matrix4(const char* uniform, mat4 matrix);
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

	void use_mesh();
};

class Material {

};

class Model {

};
