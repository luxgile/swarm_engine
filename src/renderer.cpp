#include "renderer.h"
#include "core.h"
#include <string>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

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

Window* RendererBackend::create_window(ivec2 size, string title) {
	Window* wnd = new Window(size, title);
	windows.push_back(wnd);
	return wnd;
}

void RendererBackend::destroy_window(Window* wnd) {
	windows.erase(std::remove(windows.begin(), windows.end(), wnd), windows.end());
}

Shader* RendererBackend::create_shader() {
	Shader* shader = new Shader();
	shaders.push_back(shader);
	return shader;
}

void RendererBackend::destroy_shader(Shader* shader) {
	shaders.erase(std::remove(shaders.begin(), shaders.end(), shader), shaders.end());
}

Material* RendererBackend::create_material() {
	Material* material = new Material();
	materials.push_back(material);
	return material;
}

void RendererBackend::destroy_material(Material* material) {
	materials.erase(std::remove(materials.begin(), materials.end(), material), materials.end());
}

Mesh* RendererBackend::create_mesh() {
	Mesh* mesh = new Mesh();
	meshes.push_back(mesh);
	return mesh;
}

void RendererBackend::destroy_mesh(Mesh* mesh) {
	meshes.erase(std::remove(meshes.begin(), meshes.end(), mesh), meshes.end());
}

Model* RendererBackend::create_model() {
	Model* model = new Model();
	models.push_back(model);
	return model;
}

void RendererBackend::destroy_model(Model* model) {
	models.erase(std::remove(models.begin(), models.end(), model), models.end());
}

Texture* RendererBackend::create_texture() {
	Texture* texture = new Texture();
	textures.push_back(texture);
	return texture;
}

void RendererBackend::destroy_texture(Texture* texture) {
	textures.erase(std::remove(textures.begin(), textures.end(), texture), textures.end());
}

Light* RendererBackend::create_light() {
	Light* light = new Light();
	lights.push_back(light);
	return light;
}

void RendererBackend::destroy_ligth(Light* point_ligth) {
	lights.erase(std::remove(lights.begin(), lights.end(), point_ligth), lights.end());
}

Camera* RendererBackend::create_camera() {
	Camera* camera = new Camera();
	cameras.push_back(camera);
	return camera;
}

void RendererBackend::destroy_camera(Camera* camera) {
	cameras.erase(std::remove(cameras.begin(), cameras.end(), camera), cameras.end());
}

Camera* RendererBackend::get_current_camera() {
	Camera* active = nullptr;
	int min_priority = 999999;
	for (auto c : cameras) {
		if (c->priority < min_priority) {
			min_priority = c->priority;
			active = c;
		}
	}
	return active;
}

Visual* RendererBackend::create_visual() {
	Visual* v = new Visual();
	visuals.push_back(v);
	return v;
}

void RendererBackend::destroy_visual(Visual* v) {
	visuals.erase(std::remove(visuals.begin(), visuals.end(), v), visuals.end());
}

void RendererBackend::draw_visuals() {
	auto camera = get_current_camera();

	glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	update_shader_globals();

	for (auto v : visuals) {
		auto mvp = camera->get_proj_mat() * camera->get_view_mat() * *v->get_xform();
		v->get_material()->use_material();
		v->get_material()->get_shader()->set_matrix4("matModel", *v->get_xform());
		v->get_material()->get_shader()->set_matrix4("mvp", mvp);

		for (auto mesh : v->get_model()->meshes) {
			mesh->use_mesh();
			glDrawElements(GL_TRIANGLES, mesh->get_elements_count(), GL_UNSIGNED_INT, 0);
		}
	}
}

void RendererBackend::update_shader_globals() {
	auto camera = get_current_camera();
	for (auto shader : shaders) {
		shader->set_vec3("viewPos", camera->get_view_mat()[3]);
		shader->set_vec3("ambientColor", ambient_color);
		shader->set_float("ambient", ambient_intensity);
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
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
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
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
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
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
	glUniform1i(uniform_loc, id);
}

void Shader::set_bool(const char* uniform, bool value) const {
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform1i(uniform_loc, (int)value);
}

void Shader::set_int(const char* uniform, int value) const {
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform1i(uniform_loc, value);
}

void Shader::set_float(const char* uniform, float value) const {
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform1f(uniform_loc, value);
}

void Shader::set_vec2(const char* uniform, vec2 value) const {
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform2f(uniform_loc, value.x, value.y);
}

void Shader::set_vec3(const char* uniform, vec3 value) const {
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform3f(uniform_loc, value.x, value.y, value.z);
}

void Shader::set_vec4(const char* uniform, vec4 value) const {
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform4f(uniform_loc, value.x, value.y, value.z, value.w);
}

void Shader::set_matrix4(const char* uniform, mat4 matrix) const {
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
	Shader* shader = render_bd->create_shader();
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

	Model* model = App::get_render_backend()->create_model();
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

	Mesh* gpu_mesh = App::get_render_backend()->create_mesh();
	gpu_mesh->set_vertices(vertices);
	gpu_mesh->set_triangles(indices);
	return gpu_mesh;
}

Texture::Texture() {
	glGenTextures(1, &gl_texture);
}

void Texture::use_texture(uint id) {
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, gl_texture);
}

void Texture::set_rgb(uint width, uint heigth, unsigned char* data) {
	glBindTexture(GL_TEXTURE_2D, gl_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
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
	glBindTexture(GL_TEXTURE_2D, gl_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_wrap);
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

	glBindTexture(GL_TEXTURE_2D, gl_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_mm_filter);
}

Texture* TextureImport::load_file(const char* path) {
	fprintf(stderr, "INFO: Loading texture at: %s\n", path);
	int width, heigth, nrChannels;
	unsigned char* data = stbi_load(path, &width, &heigth, &nrChannels, 0);
	if (!data) {
		fprintf(stderr, "ERROR: Failed to load texture at: %s\n %s\n", path, stbi_failure_reason());
		return nullptr;
	}

	Texture* texture = App::get_render_backend()->create_texture();
	texture->set_rgb(width, heigth, data);
	stbi_image_free(data);
	return nullptr;
}

Material::Material() {

}

void Material::use_material() const {
	for (size_t i = 0; i < textures.size(); i++) {
		if (textures[i] == nullptr) continue;
		textures[i]->use_texture(i);
	}
	shader->use_shader();
}
