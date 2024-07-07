#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include "utils.h"
#include "glm/common.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

typedef unsigned int GL_ID;
typedef unsigned int uint;

using namespace glm;
using namespace std;

class Window;
class Shader;
class Mesh;
class Model;
class Visual;
class Camera;
class PointLight;

class RendererBackend {
public:
	vec3 clear_color = vec3(1.0f);
	vec3 ambient_color = vec3(0.3f, 0.3f, 0.1f);
	float ambient_intensity = 0.3f;

	std::vector<Window*> windows;
	std::vector<Shader*> shaders;
	std::vector<Mesh*> meshes;
	std::vector<Model*> models;
	std::vector<Camera*> cameras;
	std::vector<PointLight*> lights;
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

	Model* create_model();
	void destroy_model(Model* model);

	PointLight* create_point_light();
	void destroy_point_ligth(PointLight* point_ligth);

	Camera* create_camera();
	void destroy_camera(Camera* camera);
	Camera* get_current_camera();

	Visual* create_visual();
	void destroy_visual(Visual* mesh);

	void draw_visuals();
	void update_shader_globals();
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
	VertexSrc,
	FragmentSrc
};
inline int to_gl_define(ShaderSrcType type);

class Shader {
	GL_ID gl_program;

private:
	unsigned int compile_source(ShaderSrcType type, const char* src);

public:
	void compile_shader(const char* vert, const char* frag);
	void use_shader() const;
	void set_bool(const char* uniform, bool value) const;
	void set_int(const char* uniform, int value) const;
	void set_float(const char* uniform, float value) const;
	void set_vec2(const char* uniform, vec2 value) const;
	void set_vec3(const char* uniform, vec3 value) const;
	void set_vec4(const char* uniform, vec4 value) const;
	void set_matrix4(const char* uniform, mat4 matrix) const;
};

struct Vertex {
	vec3 position;
	vec3 normal = vec3(0.0f, 0.0f, 0.0f);
	vec3 tangent = vec3(0.0f, 0.0f, 0.0f);
	vec3 color = vec3(1.0f, 1.0f, 1.0f);
	vec2 coords = vec2(0.0f, 0.0f);
};

class Mesh {
	GL_ID gl_vertex_array; // Holds vertex structure
	uint vertex_count;
	GL_ID gl_vertex_buffer; // Holds vertex data
	uint elements_count;
	GL_ID gl_elements_buffer; // Holds triangle data

public:
	Mesh();

	void set_triangles(vector<unsigned int> indices);
	void set_vertices(vector<Vertex> vertices);

	void use_mesh() const;
	uint get_vertex_count() { return vertex_count; }
	uint get_elements_count() { return elements_count; }
};

class Model {
public:
	vector<Mesh*> meshes;
};

class Visual {
	mat4 xform;
	Shader* shader;
	Model* model;

public:
	void set_xform(mat4 xform) { this->xform = xform; }
	const mat4* get_xform() { return &xform; }
	void set_model(Model* model) { this->model = model; }
	const Model* get_model() { return model; }
	void set_shader(Shader* shader) { this->shader = shader; }
	const Shader* get_shader() { return shader; }
};

class PointLight {
public:
	vec3 position;
	float intensity;
	vec3 color;
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
class ModelImport : FileImport<Model> {
public:
	Model* load_file(const char* path) override;
	void process_ai_node(Model* model, aiNode* node, const aiScene* scene);
	Mesh* process_ai_mesh(aiMesh* mesh, const aiScene* scene);
};
