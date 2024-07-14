#include "renderer.h"
#include "../core.h"
#include <string>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
const int SHADOW_RES = 1024;

RendererBackend::RendererBackend() {
	setup_gl();

	// Main Window
	Window* wnd = create_window({ 1280, 720 }, "Swarm Window");
	wnd->make_current();

	setup_glew();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
}

void RendererBackend::setup_gl() {
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return;
	}
}

void RendererBackend::setup_glew() {
	glewExperimental = GL_TRUE;
	glewInit();
}

void RendererBackend::setup_internals() {
	auto importer = ShaderImport();
	auto shadowmap_shader = importer.load_file("E:/dev/Swarm/res/depth");
	shadowmap_mat = materials.create();
	shadowmap_mat->set_shader(shadowmap_shader);

	shadows_fbo = frame_buffers.create();

	shadowmap_textures = texture_arrays.create();
	shadowmap_textures->set_as_depth(SHADOW_RES, SHADOW_RES, 16, NULL);
	shadowmap_textures->set_filter(TextureFilter::Linear);
	shadowmap_textures->set_wrap(TextureWrap::ClampBorder);
	shadowmap_textures->set_border_color(vec4(1.0, 1.0, 1.0, 1.0));
}

Window* RendererBackend::create_window(ivec2 size, string title) {
	Window* wnd = new Window(size, title);
	windows.push_back(wnd);
	return wnd;
}

void RendererBackend::destroy_window(Window* wnd) {
	windows.erase(std::remove(windows.begin(), windows.end(), wnd), windows.end());
}

void RendererBackend::render_worlds() {
	for (auto w : worlds) {
		render_world(w);
	}
}

void RendererBackend::render_world(RenderWorld* world) {
	auto camera = world->get_active_camera();
	render_shadowmaps(world->lights, world->visuals);
	update_material_globals(world);

	auto ccolor = world->env->clear_color;
	glClearColor(ccolor.r, ccolor.g, ccolor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_skybox(world);
	render_visuals(camera->get_proj_mat(), camera->get_view_mat(), world->visuals, nullptr);
}

void RendererBackend::render_shadowmaps(vector<Light*> lights, vector<Visual*> visuals) {

	for (size_t i = 0; i < lights.size(); i++) {
		auto light = lights[i];
		if (!light->get_cast_shadows()) continue;

		auto proj = light->build_proj_matrix();
		auto view = light->build_view_matrix();

		//shadows_fbo->set_output_depth(sm->shadowmap);
		shadows_fbo->set_output_depth(shadowmap_textures, i);
		if (!shadows_fbo->is_complete()) {
			fprintf(stderr, "ERROR: Shadowmap frame buffer is incompleted. Some shadows might be missing");
			continue;
		}
		glViewport(0, 0, SHADOW_RES, SHADOW_RES);
		shadows_fbo->use_framebuffer();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//sm->shadowmap->activate(SamplerID::Albedo);
		shadowmap_textures->activate(SamplerID::Albedo);
		render_visuals(proj, view, visuals, shadowmap_mat);

		FrameBuffer::unbind_framebuffer();
		glViewport(0, 0, 1280, 720);
	}

}

void RendererBackend::render_skybox(RenderWorld* world) {

	auto camera = world->get_active_camera();
	auto view = mat4(mat3(camera->get_view_mat())); // Crop out position
	auto proj = camera->get_proj_mat();

	auto env = world->env;
	auto skybox = env->skybox;

	skybox->get_material()->get_shader()->set_matrix4("projection", proj);
	skybox->get_material()->get_shader()->set_matrix4("view", view);

	glCullFace(GL_FRONT);
	glDepthMask(GL_FALSE);
	render_visual(skybox);
	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);
}

void RendererBackend::render_visuals(mat4 proj, mat4 view, vector<Visual*> visuals, Material* mat_override = nullptr) {
	for (auto v : visuals) {
		auto mvp = proj * view * *v->get_xform();
		auto mat = mat_override ? mat_override : v->get_material();
		mat->get_shader()->set_matrix4("matModel", *v->get_xform());
		mat->get_shader()->set_matrix4("mvp", mvp);
		render_visual(mat, v->get_model());
	}
}

void RendererBackend::render_visual(Visual* visual) {
	render_visual(visual->get_material(), visual->get_model());
}

void RendererBackend::render_visual(Material* material, Model* model) {
	material->use_material();

	for (auto mesh : model->meshes) {
		mesh->use_mesh();
		glDrawElements(GL_TRIANGLES, mesh->get_elements_count(), GL_UNSIGNED_INT, 0);
	}
}

void RendererBackend::update_material_globals(RenderWorld* world) {
	auto materials = world->materials;
	auto lights = world->lights;
	auto camera = world->get_active_camera();
	auto env = world->env;
	for (auto material : materials) {
		material->update_internals();

		auto shader = material->get_shader();
		shader->set_vec3("viewPos", glm::inverse(camera->get_view_mat())[3]);
		shader->set_vec3("ambientColor", env->ambient_color);
		shader->set_float("ambient", env->ambient_intensity);
		shader->set_int("numOfLights", lights.size());
		for (size_t i = 0; i < lights.size(); i++) {
			auto light = lights[i];
			string ligth_access_std = "lights[" + std::to_string(i) + "].";
			shader->set_bool((ligth_access_std + "enabled").c_str(), true);
			shader->set_int((ligth_access_std + "type").c_str(), (int)light->type);
			shader->set_vec3((ligth_access_std + "position").c_str(), light->position);
			shader->set_vec3((ligth_access_std + "direction").c_str(), light->dir);
			shader->set_vec3((ligth_access_std + "color").c_str(), light->color);
			shader->set_float((ligth_access_std + "intensity").c_str(), light->intensity);

			if (light->get_cast_shadows()) {
				shader->set_matrix4("matLight[" + std::to_string(i) + "]", light->build_proj_matrix() * light->build_view_matrix());
				material->set_texture(SamplerID::Shadows, shadowmap_textures);
			}
		}
	}
}

Window::Window(ivec2 size, string title) {
	glfwWindowHint(GLFW_SAMPLES, 4);
	gl_wnd = glfwCreateWindow(size.x, size.y, title.c_str(), NULL, NULL);
}

void Window::make_current() {
	glfwMakeContextCurrent(gl_wnd);
}

bool Window::should_close() {
	return glfwWindowShouldClose(gl_wnd);
}

void Window::swap_buffers() {
	glfwSwapBuffers(gl_wnd);
}

unsigned int Shader::compile_source(ShaderSrcType type, const char* src) {
	unsigned int compiled = glCreateShader(to_gl_define(type));
	glShaderSource(compiled, 1, &src, NULL);
	glCompileShader(compiled);

	int success;
	glGetShaderiv(compiled, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(compiled, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	};
	return compiled;
}

void Shader::compile_shader(const char* vert, const char* frag) {
	auto vertex = compile_source(ShaderSrcType::VertexSrc, vert);
	auto fragment = compile_source(ShaderSrcType::FragmentSrc, frag);

	gl_program = glCreateProgram();
	glAttachShader(gl_program, vertex);
	glAttachShader(gl_program, fragment);
	glLinkProgram(gl_program);

	int success;
	glGetProgramiv(gl_program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(gl_program, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED:\n\t" << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::use_shader() const {
	glUseProgram(gl_program);
}

void Shader::set_sampler_id(string uniform, SamplerID id) {
	set_sampler_id(uniform, (uint)id);
}

void Shader::set_sampler_id(string uniform, uint id) {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
	glUniform1i(uniform_loc, id);
}

void Shader::set_bool(const char* uniform, bool value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform1i(uniform_loc, (int)value);
}

void Shader::set_int(const char* uniform, int value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform1i(uniform_loc, value);
}

void Shader::set_float(const char* uniform, float value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform1f(uniform_loc, value);
}

void Shader::set_vec2(const char* uniform, vec2 value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform2f(uniform_loc, value.x, value.y);
}

void Shader::set_vec3(const char* uniform, vec3 value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform3f(uniform_loc, value.x, value.y, value.z);
}

void Shader::set_vec4(const char* uniform, vec4 value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform4f(uniform_loc, value.x, value.y, value.z, value.w);
}

void Shader::set_matrix4(const char* uniform, mat4 matrix) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(matrix));
}

inline int to_gl_define(ShaderSrcType type) {
	switch (type) {
	case VertexSrc:
		return GL_VERTEX_SHADER;
	case FragmentSrc:
		return GL_FRAGMENT_SHADER;
	}
	return -1;
}

uint to_gl(TextureFormat format) {
	switch (format) {
	case RGBA8:
		return GL_RGB;
		break;
	case D24_S8:
		return GL_DEPTH24_STENCIL8;
		break;
	}
	return 0;
}

Mesh::Mesh() {
	elements_count = 0;
	vertex_count = 0;
	glGenVertexArrays(1, &gl_vertex_array);
	glGenBuffers(1, &gl_vertex_buffer);
	glGenBuffers(1, &gl_elements_buffer);
}

void Mesh::set_triangles(vector<unsigned int> indices) {
	elements_count = indices.size();
	glBindVertexArray(gl_vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_elements_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

void Mesh::set_vertices(vector<Vertex> vertices) {
	vertex_count = vertices.size();
	glBindVertexArray(gl_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (void*)0);	// VERTEX POSITION
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (void*)(sizeof(float) * 3));	// VERTEX NORMAL
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (void*)(sizeof(float) * 6));	// VERTEX TANGENT
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (void*)(sizeof(float) * 9));	// VERTEX COLOR
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (void*)(sizeof(float) * 12));	// VERTEX COORDS
	glEnableVertexAttribArray(4);
}

void Mesh::use_mesh() const {
	glBindVertexArray(gl_vertex_array);
}

void Camera::set_view(vec3 pos, vec3 target, vec3 up) {
	view = glm::lookAt(pos, target, up);
}

void Camera::set_proj(float fov, vec2 screen_size, vec2 near_far_plane) {
	proj = perspectiveFov(fov, screen_size.x, screen_size.y, near_far_plane.x, near_far_plane.y);
}

Shader* ShaderImport::load_file(const char* path) {
	fprintf(stderr, "INFO: loading shader at path: %s\n", path);
	auto vert = utils::load_text(string(path).append(".vert").c_str());
	auto frag = utils::load_text(string(path).append(".frag").c_str());

	auto render_bd = App::get_render_backend();
	Shader* shader = render_bd->shaders.create();
	shader->compile_shader(vert.c_str(), frag.c_str());
	return shader;
}

Model* ModelImport::load_file(const char* path) {
	fprintf(stderr, "INFO: loading model at path: %s\n", path);
	Assimp::Importer importer;
	auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		fprintf(stderr, "ERROR: loading model failed: %s\n", importer.GetErrorString());
	}

	Model* model = App::get_render_backend()->models.create();
	process_ai_node(model, scene->mRootNode, scene);
	return model;
}

void ModelImport::process_ai_node(Model* model, aiNode* node, const aiScene* scene) {
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		model->meshes.push_back(process_ai_mesh(mesh, scene));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		process_ai_node(model, node->mChildren[i], scene);
	}
}

struct Vertex;
Mesh* ModelImport::process_ai_mesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	//vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		if (mesh->mTangents) {
			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;
		}

		if (mesh->mTextureCoords[0]) {
			vertex.coords.x = mesh->mTextureCoords[0][i].x;
			vertex.coords.y = mesh->mTextureCoords[0][i].y;
		}
		else {
			vertex.coords = vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}
	// process indices
	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// process material
	if (mesh->mMaterialIndex >= 0) {
		//aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		//vector<Texture> diffuseMaps = loadMaterialTextures(material,
		//	aiTextureType_DIFFUSE, "texture_diffuse");
		//textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		//vector<Texture> specularMaps = loadMaterialTextures(material,
		//	aiTextureType_SPECULAR, "texture_specular");
		//textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	Mesh* gpu_mesh = App::get_render_backend()->meshes.create();
	gpu_mesh->set_vertices(vertices);
	gpu_mesh->set_triangles(indices);
	return gpu_mesh;
}

Texture2D::Texture2D() {
	glGenTextures(1, &gl_texture);
}

void Texture2D::activate(uint id) {
	glActiveTexture(GL_TEXTURE0 + id);
	use_texture();
}

void Texture2D::use_texture() {
	glBindTexture(GL_TEXTURE_2D, gl_texture);
}

void Texture2D::set_as_depth(uint width, uint heigth, unsigned char* data) {
	glBindTexture(GL_TEXTURE_2D, gl_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, heigth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture2D::set_as_rgb8(uint width, uint heigth, unsigned char* data) {
	glBindTexture(GL_TEXTURE_2D, gl_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

uint Texture2D::get_gl_type() const {
	return GL_TEXTURE_2D;
}

Texture2D* Texture2DImport::load_file(const char* path) {
	fprintf(stderr, "INFO: Loading texture at: %s\n", path);
	int width, heigth, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &heigth, &nrChannels, 0);
	if (!data) {
		fprintf(stderr, "ERROR: Failed to load texture at: %s\n %s\n", path, stbi_failure_reason());
		return nullptr;
	}

	Texture2D* texture = App::get_render_backend()->textures.create();
	texture->set_as_rgb8(width, heigth, data);
	stbi_image_free(data);
	return texture;
}

Material::Material() {

}

void Material::use_material() const {
	for (size_t i = 0; i < textures.size(); i++) {
		if (textures[i] == nullptr) continue;
		textures[i]->activate(i);
	}
	shader->use_shader();
}

void FrameBuffer::set_format_2D(uint attachment, uint texture_type, GL_ID id) {
	use_framebuffer();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture_type, id, 0);
	unbind_framebuffer();
}

void FrameBuffer::set_format_3D(uint attachment, uint texture_type, uint layer, GL_ID id) {
	use_framebuffer();
	glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, id, 0, layer);
	unbind_framebuffer();
}

FrameBuffer::FrameBuffer() {
	glGenFramebuffers(1, &gl_fbo);
}

void FrameBuffer::unbind_framebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool FrameBuffer::is_complete() {
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void FrameBuffer::use_framebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, gl_fbo);
}

void FrameBuffer::use_read() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gl_fbo);
}

void FrameBuffer::use_draw() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl_fbo);
}

void FrameBuffer::set_output_depth(Texture2D* texture) {
	set_format_2D(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->get_gl_id());
}

void FrameBuffer::set_output_depth(Texture2DArray* texture, uint layer) {
	set_format_3D(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_ARRAY, layer, texture->get_gl_id());
}

void FrameBuffer::set_output_depth(RenderBuffer* rbo) {
	set_format_2D(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rbo->get_gl_id());
}

void FrameBuffer::set_output_depth_stencil(Texture2D* texture) {
	set_format_2D(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture->get_gl_id());
}

void FrameBuffer::set_output_depth_stencil(RenderBuffer* rbo) {
	set_format_2D(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, rbo->get_gl_id());
}

void FrameBuffer::set_output_color(Texture2D* texture, uint id = 0) {
	set_format_2D(GL_COLOR_ATTACHMENT0 + id, GL_TEXTURE_2D, texture->get_gl_id());
}

void FrameBuffer::set_output_color(RenderBuffer* rbo, uint id) {
	set_format_2D(GL_COLOR_ATTACHMENT0 + id, GL_TEXTURE_2D, rbo->get_gl_id());

}

RenderBuffer::RenderBuffer() {
	glGenRenderbuffers(1, &gl_rbo);
}

void RenderBuffer::set_format(TextureFormat format, vec2 size) {
	glBindRenderbuffer(GL_RENDERBUFFER, gl_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, to_gl(format), size.x, size.y);
}

mat4 Light::build_view_matrix() {
	if (type == LightType::Directional) return glm::lookAt(-dir * 10.0f, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
	return glm::lookAt(position, position + dir, vec3(0, 1, 0));
}

mat4 Light::build_proj_matrix() {
	switch (type) {
	case Directional:
		return glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 20.0f);
	case Point:
	default:
		return glm::identity<mat4>();
	}
}

void Light::set_cast_shadows(bool state) {
	cast_shadows = state;
}

Texture2DArray::Texture2DArray() {
	glGenTextures(1, &gl_texture_array);
}

GL_ID Texture2DArray::get_gl_id() const {
	return gl_texture_array;
}

void Texture2DArray::activate(uint id) {
	glActiveTexture(GL_TEXTURE0 + id);
	use_texture();
}

void Texture2DArray::use_texture() {
	glBindTexture(GL_TEXTURE_2D_ARRAY, gl_texture_array);
}

void Texture2DArray::set_as_depth(uint width, uint heigth, unsigned char* data) {
	set_as_depth(width, heigth, 1, data);
}
void Texture2DArray::set_as_depth(uint width, uint heigth, uint depth, unsigned char* data) {
	glBindTexture(GL_TEXTURE_2D_ARRAY, gl_texture_array);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, width, heigth, depth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

void Texture2DArray::set_as_rgb8(uint width, uint heigth, unsigned char* data) {
	set_as_rgb8(width, heigth, 1, data);
}

void Texture2DArray::set_as_rgb8(uint width, uint heigth, uint depth, unsigned char* data) {
	glBindTexture(GL_TEXTURE_2D_ARRAY, gl_texture_array);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, heigth, depth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

uint Texture2DArray::get_gl_type() const {
	return GL_TEXTURE_2D_ARRAY;
}

CubemapTexture::CubemapTexture() {
	glGenTextures(1, &gl_cubemap);
}

GL_ID CubemapTexture::get_gl_id() const {
	return gl_cubemap;
}

void CubemapTexture::set_as_depth(uint width, uint heigth, unsigned char* data) {
}

void CubemapTexture::set_as_rgb8(uint width, uint heigth, unsigned char* data) {
}

void CubemapTexture::set_as_rgb8(uint width, uint heigth, vector<unsigned char*> data) {
	use_texture();
	for (size_t i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, data[i]);
	}
}

uint CubemapTexture::get_gl_type() const {
	return GL_TEXTURE_CUBE_MAP;
}

void Texture::activate(uint id) {
	glActiveTexture(GL_TEXTURE0 + id);
	use_texture();
}

void Texture::use_texture() {
	glBindTexture(get_gl_type(), get_gl_id());
}

void Texture::set_wrap(TextureWrap wrap) {
	uint gl_wrap = 0;
	switch (wrap) {
	case Repeat:
		gl_wrap = GL_REPEAT;
		break;
	case Mirrored:
		gl_wrap = GL_MIRRORED_REPEAT;
		break;
	case ClampEdge:
		gl_wrap = GL_CLAMP_TO_EDGE;
		break;
	case ClampBorder:
		gl_wrap = GL_CLAMP_TO_BORDER;
		break;
	}
	auto type = get_gl_type();
	glBindTexture(type, get_gl_id());
	glTexParameteri(type, GL_TEXTURE_WRAP_S, gl_wrap);
	glTexParameteri(type, GL_TEXTURE_WRAP_T, gl_wrap);
}

void Texture::set_filter(TextureFilter filter) {
	uint gl_filter = 0;
	uint gl_mm_filter = 0;
	switch (filter) {
	case Nearest:
		gl_filter = GL_NEAREST;
		gl_mm_filter = GL_NEAREST_MIPMAP_NEAREST;
		break;
	case Linear:
		gl_filter = GL_LINEAR;
		gl_mm_filter = GL_LINEAR_MIPMAP_LINEAR;
		break;
	}

	auto type = get_gl_type();
	glBindTexture(type, get_gl_id());
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, gl_filter);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, gl_mm_filter);
}

void Texture::set_border_color(vec4 color) {
	use_texture();
	glTexParameterfv(get_gl_type(), GL_TEXTURE_BORDER_COLOR, &color.x);
}

CubemapTexture* CubemapTextureImport::load_file(const char* path) {
	fprintf(stderr, "INFO: Loading cubemap at: %s\n", path);
	int width, heigth, nrChannels;
	vector<unsigned char*> cubemap_data;
	for (size_t i = 0; i < 6; i++) {
		auto face_path = string(path);
		std::replace(face_path.begin(), face_path.end(), '#', std::to_string(i).c_str()[0]);
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(face_path.c_str(), &width, &heigth, &nrChannels, 0);
		if (!data) {
			fprintf(stderr, "ERROR: Failed to load texture at: %s\n %s\n", face_path.c_str(), stbi_failure_reason());
			return nullptr;
		}
		fprintf(stderr, "\tLoading cubemap face at : %s\n", face_path.c_str());
		cubemap_data.push_back(data);
	}

	auto cubemap = App::get_render_backend()->cubemaps.create();
	cubemap->set_as_rgb8(width, heigth, cubemap_data);

	for (auto d : cubemap_data) {
		stbi_image_free(d);
	}

	return cubemap;
}

void PbrMaterial::update_internals() {
	auto shader = get_shader();
	shader->set_vec4("albedoColor", albedo);
	shader->set_vec4("emissiveColor", emissive);
	shader->set_float("metallicValue", metallic);
	shader->set_float("roughnessValue", roughness);
	shader->set_float("aoValue", ambient_occlusion);
}