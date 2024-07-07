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
class Light;
class Texture;
class Material;

class RendererBackend {
public:
	vec3 clear_color = vec3(1.0f);
	vec3 ambient_color = vec3(0.3f, 0.3f, 0.1f);
	float ambient_intensity = 0.3f;

	std::vector<Window*> windows;
	std::vector<Shader*> shaders;
	std::vector<Mesh*> meshes;
	std::vector<Model*> models;
	std::vector<Texture*> textures;
	std::vector<Camera*> cameras;
	std::vector<Light*> lights;
	std::vector<Material*> materials;
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

	Material* create_material();
	void destroy_material(Material* material);

	Mesh* create_mesh();
	void destroy_mesh(Mesh* mesh);

	Model* create_model();
	void destroy_model(Model* model);

	Texture* create_texture();
	void destroy_texture(Texture* texture);

	Light* create_light();
	void destroy_ligth(Light* light);

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
enum SamplerID {
	Albedo = 0,
	Normal,
	MRA,
	Emissive,
};

class Shader {
	GL_ID gl_program;

private:
	unsigned int compile_source(ShaderSrcType type, const char* src);

public:
	void compile_shader(const char* vert, const char* frag);
	void use_shader() const;
	void set_sampler_id(string uniform, SamplerID id);
	void set_sampler_id(string uniform, uint id);
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

enum TextureWrap {
	Repeat,
	Mirrored,
	ClampEdge,
	ClampBorder,
};
enum TextureFilter {
	Nearest,
	Linear,
};
class Texture {
	GL_ID gl_texture;

public:
	Texture();

public:
	void use_texture(uint id);
	void set_rgb(uint width, uint heigth, unsigned char* data);
	void set_wrap(TextureWrap wrap);
	void set_filter(TextureFilter wrap);
};

class Model {
public:
	vector<Mesh*> meshes;
};

class Material {
	Shader* shader;
	vector<Texture*> textures = vector<Texture*>(16, nullptr);
public:
	Material();
	/// @brief Tell GL to render using this material.
	void use_material() const;
	void set_shader(Shader* shader) { this->shader = shader; }
	Shader* get_shader() { return shader; }
	void set_texture(uint id, Texture* texture) { this->textures[id] = texture; }
	void set_texture(SamplerID id, Texture* texture) { this->textures[id] = texture; }
};

class Visual {
	mat4 xform;
	Material* material;
	Model* model;

public:
	void set_xform(mat4 xform) { this->xform = xform; }
	mat4* get_xform() { return &xform; }
	void set_model(Model* model) { this->model = model; }
	Model* get_model() { return model; }
	void set_material(Material* shader) { this->material = shader; }
	Material* get_material() { return material; }
};

enum LightType {
	Point = 0,
	Directional,
};
struct Light {
	LightType type;
	vec3 dir;
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
class TextureImport : FileImport<Texture> {
public:
	Texture* load_file(const char* path) override;
};
