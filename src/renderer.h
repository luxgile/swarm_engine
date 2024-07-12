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
#include "MemPool.h"

typedef unsigned int GL_ID;
typedef unsigned int uint;

using namespace glm;
using namespace std;

enum RenderMode {
	Forward,
	Deferred,
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
	Shadows,
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
	void set_matrix4(const string uniform, mat4 matrix) const { set_matrix4(uniform.c_str(), matrix); }
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

enum TextureFormat {
	RGBA8,
	D24_S8,
};
uint to_gl(TextureFormat format);
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
public:
	virtual GL_ID get_gl_id() const = 0;
	virtual	void activate(uint id) = 0;
	virtual void use_texture() = 0;
	virtual void set_as_depth(uint width, uint heigth, unsigned char* data) = 0;
	virtual void set_as_rgb8(uint width, uint heigth, unsigned char* data) = 0;
	virtual void set_wrap(TextureWrap wrap) = 0;
	virtual void set_filter(TextureFilter wrap) = 0;
	virtual void set_border_color(vec4 color) = 0;
};

class Texture2D : public Texture {
	GL_ID gl_texture;

public:
	Texture2D();

public:
	GL_ID get_gl_id() const override { return gl_texture; }
	void activate(uint id) override;
	void use_texture() override;
	void set_as_depth(uint width, uint heigth, unsigned char* data) override;
	void set_as_rgb8(uint width, uint heigth, unsigned char* data) override;
	void set_wrap(TextureWrap wrap) override;
	void set_filter(TextureFilter wrap) override;
	void set_border_color(vec4 color) override;
};


/// @brief Used with FrameBuffers for fast offscreen rendering. Only drawback is that it's not possible to read from them.
class RenderBuffer {
	GL_ID gl_rbo;

public:
	RenderBuffer();

	GL_ID get_gl_id() { return gl_rbo; }
	void set_format(TextureFormat format, vec2 size);
};

class Texture2DArray : public Texture {
	GL_ID gl_texture_array;

public:
	Texture2DArray();

	// Heredado vía Texture
	GL_ID get_gl_id() const override;
	void activate(uint id) override;
	void use_texture() override;
	void set_as_depth(uint width, uint heigth, unsigned char* data) override;
	void set_as_depth(uint width, uint heigth, uint depth, unsigned char* data);
	void set_as_rgb8(uint width, uint heigth, unsigned char* data) override;
	void set_as_rgb8(uint width, uint heigth, uint depth, unsigned char* data);
	void set_wrap(TextureWrap wrap) override;
	void set_filter(TextureFilter wrap) override;
	void set_border_color(vec4 color) override;
};



/// @brief When used, all render operations are drawn into the frame buffer. Needs a texture or render buffer attached as output.
class FrameBuffer {
	GL_ID gl_fbo;

	void set_format_2D(uint attachment, uint texture_type, GL_ID id);
	void set_format_3D(uint attachment, uint texture_type, uint layer, GL_ID id);

public:
	FrameBuffer();

	static void unbind_framebuffer();
	bool is_complete();
	void use_framebuffer();
	void use_read();
	void use_draw();
	void set_output_depth(Texture2D* texture);
	void set_output_depth(Texture2DArray* texture, uint layer);
	void set_output_depth(RenderBuffer* rbo);
	void set_output_depth_stencil(Texture2D* texture);
	void set_output_depth_stencil(RenderBuffer* rbo);
	void set_output_color(Texture2D* texture, uint id);
	void set_output_color(RenderBuffer* rbo, uint id);
};

class CubemapTexture {

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

struct ShadowMap {
	Texture2D* shadowmap;

	ShadowMap();
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

	mat4 build_view_matrix();
	mat4 build_proj_matrix();

	void set_cast_shadows(bool state);
	bool get_cast_shadows() { return cast_shadows; }
	ShadowMap* get_shadow_map() const;

private:
	ShadowMap* shadow_map;
	bool cast_shadows;
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
class TextureImport : FileImport<Texture2D> {
public:
	Texture2D* load_file(const char* path) override;
};

class RendererBackend {
private:
	FrameBuffer* shadows_fbo;
	Material* shadowmap_mat;
	Texture2DArray* shadowmap_textures;

public:
	vec3 clear_color = vec3(1.0f);
	vec3 ambient_color = vec3(0.3f, 0.3f, 0.1f);
	float ambient_intensity = 0.3f;
	RenderMode mode = RenderMode::Forward;

	std::vector<Window*> windows;
	MemPool<Shader> shaders;
	MemPool<Mesh> meshes;
	MemPool<Model> models;
	MemPool<Texture2D> textures;
	MemPool<Texture2DArray> texture_arrays;
	MemPool<RenderBuffer> render_buffers;
	MemPool<FrameBuffer> frame_buffers;
	MemPool<Camera> cameras;
	MemPool<Light> lights;
	MemPool<ShadowMap> shadow_maps;
	MemPool<Material> materials;
	MemPool<Visual> visuals;

private:
	void setup_gl();
	void setup_glew();

	void render_visuals_forward();
	void render_shadowmaps();

	void render_visuals_deferred();

	void render_visuals(mat4 proj, mat4 view, Material* mat_override);

public:
	RendererBackend();
	void setup_internals();

	Window* create_window(ivec2 size, string title);
	void destroy_window(Window* wnd);

	void render_frame();
	void update_material_globals();

	Camera* get_active_camera();
};
