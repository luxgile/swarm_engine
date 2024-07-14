#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include "../utils.h"
#include "glm/common.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../MemPool.h"
#include "render_world.h"

typedef unsigned int GL_ID;
typedef unsigned int uint;

using namespace glm;
using namespace std;

class Viewport;
class RenderEnviroment;
class RenderWorld;

class Window {
	Viewport* vp;

public:
	GLFWwindow* gl_wnd;

public:
	Window(ivec2 size, string title);
	void make_current();
	bool should_close();
	void swap_buffers();

	void set_viewport(Viewport* vp);
	const Viewport* get_viewport() { return vp; }

	void set_size(ivec2 size);
	ivec2 get_size();
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
	Skybox,
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
	uint get_vertex_count() const { return vertex_count; }
	uint get_elements_count() const { return elements_count; }
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
	virtual uint get_gl_type() const = 0;
	virtual GL_ID get_gl_id() const = 0;
	virtual	void activate(uint id);
	virtual void use_texture();
	virtual void set_as_depth(uint width, uint heigth, unsigned char* data) = 0;
	virtual void set_as_rgb8(uint width, uint heigth, unsigned char* data) = 0;
	virtual void set_as_depth_stencil(uint width, uint heigth, unsigned char* data);
	virtual void set_wrap(TextureWrap wrap);
	virtual void set_filter(TextureFilter wrap);
	virtual void set_border_color(vec4 color);
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

	// Inherited via Texture
	uint get_gl_type() const override;
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

	// Inherited via Texture
	uint get_gl_type() const override;
};



/// @brief When used, all render operations are drawn into the frame buffer. Needs a texture or render buffer attached as output.
class FrameBuffer {
	GL_ID gl_fbo;

	void set_format_2D(uint attachment, uint texture_type, GL_ID id);
	void set_format_3D(uint attachment, uint texture_type, uint layer, GL_ID id);

public:
	FrameBuffer();

	static void unbind_framebuffer();
	GL_ID get_gl_id() { return gl_fbo; }
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

class CubemapTexture : public Texture {
	GL_ID gl_cubemap;

public:
	CubemapTexture();

	GL_ID get_gl_id() const override;
	uint get_gl_type() const override;
	void set_as_depth(uint width, uint heigth, unsigned char* data) override;
	void set_as_rgb8(uint width, uint heigth, unsigned char* data) override;
	void set_as_rgb8(uint width, uint heigth, vector<unsigned char*> data);
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
	virtual void update_internals() {}
};

class PbrMaterial : public Material {
public:
	vec4 albedo = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 emissive = vec4(0.0, 0.0, 0.0, 0.0);
	float metallic = 0.3f;
	float roughness = 0.1f;
	float ambient_occlusion = 1.0f;

	void update_internals() override;
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

	mat4 build_view_matrix();
	mat4 build_proj_matrix();

	void set_cast_shadows(bool state);
	bool get_cast_shadows() { return cast_shadows; }

private:
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
class Texture2DImport : FileImport<Texture2D> {
public:
	Texture2D* load_file(const char* path) override;
};
class CubemapTextureImport : FileImport<CubemapTexture> {
public:
	CubemapTexture* load_file(const char* path) override;
};

class RendererBackend {
private:
	FrameBuffer* shadows_fbo;
	Material* shadowmap_mat;
	Texture2DArray* shadowmap_textures;

	bool imgui_installed;

public:

	std::vector<Window*> windows;
	MemPool<RenderWorld> worlds;
	MemPool<Viewport> viewports;
	MemPool<RenderEnviroment> enviroments;
	MemPool<Shader> shaders;
	MemPool<Mesh> meshes;
	MemPool<Texture2D> textures;
	MemPool<Texture2DArray> texture_arrays;
	MemPool<CubemapTexture> cubemaps;
	MemPool<RenderBuffer> render_buffers;
	MemPool<FrameBuffer> frame_buffers;
	MemPool<Light> lights;
	MemPool<Model> models;
	MemPool<Camera> cameras;
	MemPool<Material> materials;
	MemPool<Visual> visuals;


private:
	void setup_gl();
	void setup_glew();
	void setup_internals();
	void setup_imgui();

	void render_shadowmaps(vector<Light*> lights, vector<Visual*> visuals);
	void render_skybox(RenderWorld* world);
	void render_visuals(mat4 proj, mat4 view, vector<Visual*> visuals, Material* mat_override);
	void render_visual(Material* material, Model* model);
	void update_material_globals(RenderWorld* world);
	void render_visual(Visual* visual);


public:
	RendererBackend();
	void setup();
	bool is_imgui_installed() { return imgui_installed; }

	Window* get_main_window() { return windows[0]; }

	Window* create_window(ivec2 size, string title);
	Window* get_window_from_glfw(GLFWwindow* wnd);
	void destroy_window(Window* wnd);

	void debug_backend(RenderWorld* world);

	void render_worlds();
	void render_world(RenderWorld* world);
};
