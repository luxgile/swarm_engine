#include "renderer.h"
#include "core.h"
#include <string>

using namespace std;

RendererBackend::RendererBackend() {
	windows = std::vector<Window*>();

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

Mesh* RendererBackend::create_mesh() {
	Mesh* mesh = new Mesh();
	meshes.push_back(mesh);
	return mesh;
}

void RendererBackend::destroy_mesh(Mesh* mesh) {
	meshes.erase(std::remove(meshes.begin(), meshes.end(), mesh), meshes.end());
}

Window::Window(ivec2 size, string title) {
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

Shader* ShaderImport::load_file(const char* path) {
	fprintf(stderr, "INFO: loading shader at path: %s\n", path);
	auto vert = utils::load_text(string(path).append(".vert").c_str());
	auto frag = utils::load_text(string(path).append(".frag").c_str());

	auto render_bd = App::get_render_backend();
	Shader* shader = render_bd->create_shader();
	shader->compile_shader(vert.c_str(), frag.c_str());
	return shader;
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
	auto vertex = compile_source(ShaderSrcType::Vertex, vert);
	auto fragment = compile_source(ShaderSrcType::Fragment, frag);

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

void Shader::use_shader() {
	glUseProgram(gl_program);
}

inline int to_gl_define(ShaderSrcType type) {
	switch (type) {
	case Vertex:
		return GL_VERTEX_SHADER;
	case Fragment:
		return GL_FRAGMENT_SHADER;
	}
	return -1;
}

Mesh::Mesh() {
	glGenVertexArrays(1, &gl_vertex_array);
	glGenBuffers(1, &gl_vertex_buffer);
	glGenBuffers(1, &gl_elements_buffer);
}

void Mesh::set_triangles(vector<unsigned int> indices) {
	glBindVertexArray(gl_vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_elements_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

void Mesh::set_vertices(vector<vec3> vertices) {
	glBindVertexArray(gl_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * vertices.size(), &vertices[0].x, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);	// VERTEX POSITION
	glEnableVertexAttribArray(0);
}

void Mesh::use_mesh() {
	glBindVertexArray(gl_vertex_array);
}
