#include "renderer.h"
#include "../core.h"
#include <string>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include <print>

using namespace std;
const int SHADOW_RES = 1024;

RendererBackend::RendererBackend() {
}

Result<void, RendererError> RendererBackend::setup() {
	setup_gl();

	// Main Window
	AppWindow* wnd = create_window({ 1280, 720 }, "Swarm Window");
	wnd->make_current();
	wnd->maximize();

	auto rglew = setup_glew();
	if (!rglew) return rglew;

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	auto rinternals = setup_internals();
	if (!rinternals) return rinternals;

	auto rimgui = setup_imgui();
	if (!rimgui) return rimgui;
}

Result<void, RendererError> RendererBackend::setup_gl() {
	if (!glfwInit()) {
		return Error(RendererError{ .error = "ERROR: could not start GLFW3" });
	}
}

Result<void, RendererError> RendererBackend::setup_glew() {
	glewExperimental = GL_TRUE;
	if (glewInit() != 0) {
		return Error(RendererError{ .error = "ERROR: could not start GLEW" });
	}
}

Result<void, RendererError> RendererBackend::setup_internals() {
	auto rshadowmap_shader = App::get_asset_backend()->load_file<GPUShader>("depth");
	if (!rshadowmap_shader) { return Error(RendererError{ .error = "Failed to load the depth shader." }); }
	auto shadowmap_shader = rshadowmap_shader.value();
	shadowmap_mat = materials.create();
	shadowmap_mat->set_shader(shadowmap_shader);

	shadows_fbo = frame_buffers.create();

	shadowmap_textures = texture_arrays.create();
	shadowmap_textures->set_as_depth(SHADOW_RES, SHADOW_RES, 16, NULL);
	shadowmap_textures->set_filter(TextureFilter::Linear);
	shadowmap_textures->set_wrap(TextureWrap::ClampBorder);
	shadowmap_textures->set_border_color(vec4(1.0, 1.0, 1.0, 1.0));
}

Result<void, RendererError> RendererBackend::setup_imgui() {
	if (imgui_installed) return Error(RendererError{ .error = "Error: ImGui already installed." });

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplOpenGL3_Init();
	ImGui_ImplGlfw_InitForOpenGL(get_main_window()->gl_wnd, true); // We need to initialize 

	imgui_installed = true;
}

AppWindow* RendererBackend::create_window(ivec2 size, string title) {
	AppWindow* wnd = new AppWindow(size, title);
	windows.push_back(wnd);
	return wnd;
}

AppWindow* RendererBackend::get_window_from_glfw(GLFWwindow* wnd) {
	for (auto w : windows) {
		if (w->gl_wnd == wnd) return w;
	}
	return nullptr;
}

void RendererBackend::destroy_window(AppWindow* wnd) {
	windows.erase(std::remove(windows.begin(), windows.end(), wnd), windows.end());
}

void RendererBackend::debug_backend(RenderWorld* world) {
	if (!is_imgui_installed()) return;

	bool active = true;
	world->on_ui_pass.connect([this, &active]() {
		ImGui::Begin("Render Backend", &active);
		for (auto m : materials) {
			if (auto material = static_cast<GPUPbrMaterial*>(m)) {
				ImGui::ColorEdit4("Color", &material->albedo.x);
				ImGui::SliderFloat("Metallic", &material->metallic, 0, 1);
				ImGui::SliderFloat("Roughness", &material->roughness, 0, 1);
			}
		}
		ImGui::End();
	});
}

void RendererBackend::render_worlds() {
	for (auto w : worlds) {
		if (!w->is_ready()) continue;
		auto result = render_world(w);
		if (!result) std::println("{}", result.error().error);
	}
}

Result<void, RendererError> RendererBackend::render_world(RenderWorld* world) {
	if (!world->is_ready()) return Error(RendererError{ .error = "Error: World not ready to be rendered. Check it was initialized properly." });

	auto camera = world->get_active_camera();
	render_shadowmaps(world->lights, world->visuals);
	update_material_globals(world);

	if (world->vp) world->vp.value()->use_viewport();
	auto ccolor = world->env ? world->env.value()->clear_color : vec4(0, 0, 0, 0);
	glClearColor(ccolor.r, ccolor.g, ccolor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	world->on_pre_render();

	if (camera) {
		render_skybox(world);
		render_visuals(camera.value()->get_proj_mat(), camera.value()->get_view_mat(), world->visuals, nullptr);
	}

	if (is_imgui_installed() && world->imgui_draw_cmd) {
		//ImGui_ImplOpenGL3_NewFrame();
		//ImGui_ImplGlfw_NewFrame();
		//ImGui::NewFrame();

		// UI Rendering
		//world->on_ui_pass();

		//ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(world->imgui_draw_cmd.value());
	}

	world->on_post_render();

	GPUFrameBuffer::unbind_framebuffer();
}

void RendererBackend::render_shadowmaps(vector<Light*> lights, vector<GPUVisual*> visuals) {

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

		GPUFrameBuffer::unbind_framebuffer();
	}

}

void RendererBackend::render_skybox(RenderWorld* world) {

	auto opt_camera = world->get_active_camera();
	if (!opt_camera) return;
	if (!world->env) return;

	auto camera = opt_camera.value();
	auto view = mat4(mat3(camera->get_view_mat()));
	auto proj = camera->get_proj_mat();

	auto env = world->env;
	auto skybox = env.value()->skybox;

	skybox->get_material()->get_shader()->set_matrix4("projection", proj);
	skybox->get_material()->get_shader()->set_matrix4("view", view);

	glCullFace(GL_FRONT);
	glDepthMask(GL_FALSE);
	render_visual(skybox);
	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);
}

void RendererBackend::render_visuals(mat4 proj, mat4 view, vector<GPUVisual*> visuals, GPUMaterial* mat_override = nullptr) {
	for (auto v : visuals) {
		auto mvp = proj * view * *v->get_xform();
		auto mat = mat_override ? mat_override : v->get_material();
		mat->get_shader()->set_matrix4("matModel", *v->get_xform());
		mat->get_shader()->set_matrix4("mvp", mvp);
		render_visual(mat, v->get_model());
	}
}

void RendererBackend::render_visual(GPUVisual* visual) {
	render_visual(visual->get_material(), visual->get_model());
}

void RendererBackend::render_visual(GPUMaterial* material, GPUModel* model) {
	material->use_material();

	for (auto mesh : model->meshes) {
		mesh->use_mesh();
		glDrawElements(GL_TRIANGLES, mesh->get_elements_count(), GL_UNSIGNED_INT, 0);
	}
}

void RendererBackend::update_material_globals(RenderWorld* world) {
	auto materials = world->materials;
	auto lights = world->lights;
	auto opt_camera = world->get_active_camera();
	for (auto material : materials) {
		material->update_internals();

		auto shader = material->get_shader();
		if (opt_camera) {
			shader->set_vec3("viewPos", glm::inverse(opt_camera.value()->get_view_mat())[3]);
		}

		if (world->env) {
			auto env = world->env.value();
			shader->set_vec3("ambientColor", env->ambient_color);
			shader->set_float("ambient", env->ambient_intensity);
		}

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

AppWindow::AppWindow(ivec2 size, string title) {
	glfwWindowHint(GLFW_SAMPLES, 4);
	gl_wnd = glfwCreateWindow(size.x, size.y, title.c_str(), NULL, NULL);

	glfwSetWindowSizeCallback(gl_wnd, [](GLFWwindow* wnd, int w, int h) {
		auto window = App::get_render_backend()->get_window_from_glfw(wnd);
		if (window->vp)
			window->vp->set_size(vec2(w, h));
	});

	if (App::get_render_backend()->is_imgui_installed()) {
		ImGui_ImplGlfw_InitForOpenGL(gl_wnd, true);
	}
}

void AppWindow::make_current() {
	glfwMakeContextCurrent(gl_wnd);
}

bool AppWindow::should_close() {
	return glfwWindowShouldClose(gl_wnd);
}

void AppWindow::close() {
	glfwSetWindowShouldClose(gl_wnd, true);
}

void AppWindow::swap_buffers() {
	if (vp) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, vp->fbo->get_gl_id());
		auto size = get_size();
		glBlitFramebuffer(0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	glfwSwapBuffers(gl_wnd);
}

void AppWindow::set_viewport(Viewport* vp) {
	this->vp = vp;
	vp->set_size(get_size());
}

void AppWindow::maximize() {
	glfwMaximizeWindow(gl_wnd);
}

void AppWindow::set_size(ivec2 size) {
	glfwSetWindowSize(gl_wnd, size.x, size.y);
	vp->set_size(size);
}

ivec2 AppWindow::get_size() {
	ivec2 size;
	glfwGetWindowSize(gl_wnd, &size.x, &size.y);
	return size;
}

Result<GL_ID, ShaderError> GPUShader::compile_source(ShaderSrcType type, const char* src) {
	unsigned int compiled = glCreateShader(to_gl_define(type));
	glShaderSource(compiled, 1, &src, NULL);
	glCompileShader(compiled);

	int success;
	glGetShaderiv(compiled, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(compiled, 512, NULL, infoLog);
		return Error(ShaderError{ .error = format("Error: Compilation failed for shader source {}: \n%s", (int)type, infoLog) });
	};
	return compiled;
}

Result<void, ShaderError> GPUShader::compile_shader(const char* vert, const char* frag) {
	auto rvertex = compile_source(ShaderSrcType::VertexSrc, vert);
	if (!rvertex) { return Error(rvertex.error()); }
	auto vertex = rvertex.value();

	auto rfragment = compile_source(ShaderSrcType::FragmentSrc, frag);
	if (!rfragment) { return Error(rfragment.error()); }
	auto fragment = rfragment.value();

	gl_program = glCreateProgram();
	glAttachShader(gl_program, vertex);
	glAttachShader(gl_program, fragment);
	glLinkProgram(gl_program);

	int success;
	glGetProgramiv(gl_program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(gl_program, 512, NULL, infoLog);
		return Error(ShaderError{ .error = format("ERROR: Failed linking shader:\n%s", infoLog) });
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void GPUShader::use_shader() const {
	glUseProgram(gl_program);
}

void GPUShader::set_sampler_id(string uniform, SamplerID id) {
	set_sampler_id(uniform, (uint)id);
}

void GPUShader::set_sampler_id(string uniform, uint id) {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
	glUniform1i(uniform_loc, id);
}

void GPUShader::set_bool(const char* uniform, bool value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform1i(uniform_loc, (int)value);
}

void GPUShader::set_int(const char* uniform, int value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform1i(uniform_loc, value);
}

void GPUShader::set_float(const char* uniform, float value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform1f(uniform_loc, value);
}

void GPUShader::set_vec2(const char* uniform, vec2 value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform2f(uniform_loc, value.x, value.y);
}

void GPUShader::set_vec3(const char* uniform, vec3 value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform3f(uniform_loc, value.x, value.y, value.z);
}

void GPUShader::set_vec4(const char* uniform, vec4 value) const {
	use_shader();
	unsigned int uniform_loc = glGetUniformLocation(gl_program, uniform);
	glUniform4f(uniform_loc, value.x, value.y, value.z, value.w);
}

void GPUShader::set_matrix4(const char* uniform, mat4 matrix) const {
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

GPUMesh::GPUMesh() {
	elements_count = 0;
	vertex_count = 0;
	glGenVertexArrays(1, &gl_vertex_array);
	glGenBuffers(1, &gl_vertex_buffer);
	glGenBuffers(1, &gl_elements_buffer);
}

void GPUMesh::set_triangles(vector<unsigned int> indices) {
	elements_count = indices.size();
	glBindVertexArray(gl_vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_elements_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

void GPUMesh::set_vertices(vector<Vertex> vertices) {
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

void GPUMesh::use_mesh() const {
	glBindVertexArray(gl_vertex_array);
}

void Camera::set_view(vec3 pos, vec3 target, vec3 up) {
	view = glm::lookAt(pos, target, up);
}

void Camera::set_proj(float fov, vec2 screen_size, vec2 near_far_plane) {
	proj = perspectiveFov(fov, screen_size.x, screen_size.y, near_far_plane.x, near_far_plane.y);
}

GPUTexture2D::GPUTexture2D() {
	glGenTextures(1, &gl_texture);
}

void GPUTexture2D::activate(uint id) {
	glActiveTexture(GL_TEXTURE0 + id);
	use_texture();
}

void GPUTexture2D::use_texture() {
	glBindTexture(GL_TEXTURE_2D, gl_texture);
}

void GPUTexture2D::set_as_depth(uint width, uint heigth, unsigned char* data) {
	glBindTexture(GL_TEXTURE_2D, gl_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, heigth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void GPUTexture2D::set_as_rgb8(uint width, uint heigth, unsigned char* data) {
	glBindTexture(GL_TEXTURE_2D, gl_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

uint GPUTexture2D::get_gl_type() const {
	return GL_TEXTURE_2D;
}

GPUMaterial::GPUMaterial() {

}

void GPUMaterial::use_material() const {
	for (size_t i = 0; i < textures.size(); i++) {
		if (textures[i] == nullptr) continue;
		textures[i]->activate(i);
	}
	shader->use_shader();
}

void GPUFrameBuffer::set_format_2D(uint attachment, uint texture_type, GL_ID id) {
	use_framebuffer();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture_type, id, 0);
	unbind_framebuffer();
}

void GPUFrameBuffer::set_format_3D(uint attachment, uint texture_type, uint layer, GL_ID id) {
	use_framebuffer();
	glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, id, 0, layer);
	unbind_framebuffer();
}

GPUFrameBuffer::GPUFrameBuffer() {
	glGenFramebuffers(1, &gl_fbo);
}

void GPUFrameBuffer::unbind_framebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool GPUFrameBuffer::is_complete() {
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void GPUFrameBuffer::use_framebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, gl_fbo);
}

void GPUFrameBuffer::use_read() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gl_fbo);
}

void GPUFrameBuffer::use_draw() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl_fbo);
}

void GPUFrameBuffer::set_output_depth(GPUTexture2D* texture) {
	set_format_2D(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->get_gl_id());
}

void GPUFrameBuffer::set_output_depth(GPUTexture2DArray* texture, uint layer) {
	set_format_3D(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_ARRAY, layer, texture->get_gl_id());
}

void GPUFrameBuffer::set_output_depth(GPURenderBuffer* rbo) {
	set_format_2D(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rbo->get_gl_id());
}

void GPUFrameBuffer::set_output_depth_stencil(GPUTexture2D* texture) {
	set_format_2D(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture->get_gl_id());
}

void GPUFrameBuffer::set_output_depth_stencil(GPURenderBuffer* rbo) {
	set_format_2D(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, rbo->get_gl_id());
}

void GPUFrameBuffer::set_output_color(GPUTexture2D* texture, uint id = 0) {
	set_format_2D(GL_COLOR_ATTACHMENT0 + id, GL_TEXTURE_2D, texture->get_gl_id());
}

void GPUFrameBuffer::set_output_color(GPURenderBuffer* rbo, uint id) {
	set_format_2D(GL_COLOR_ATTACHMENT0 + id, GL_TEXTURE_2D, rbo->get_gl_id());

}

GPURenderBuffer::GPURenderBuffer() {
	glGenRenderbuffers(1, &gl_rbo);
}

void GPURenderBuffer::set_format(TextureFormat format, vec2 size) {
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

GPUTexture2DArray::GPUTexture2DArray() {
	glGenTextures(1, &gl_texture_array);
}

GL_ID GPUTexture2DArray::get_gl_id() const {
	return gl_texture_array;
}

void GPUTexture2DArray::activate(uint id) {
	glActiveTexture(GL_TEXTURE0 + id);
	use_texture();
}

void GPUTexture2DArray::use_texture() {
	glBindTexture(GL_TEXTURE_2D_ARRAY, gl_texture_array);
}

void GPUTexture2DArray::set_as_depth(uint width, uint heigth, unsigned char* data) {
	set_as_depth(width, heigth, 1, data);
}
void GPUTexture2DArray::set_as_depth(uint width, uint heigth, uint depth, unsigned char* data) {
	glBindTexture(GL_TEXTURE_2D_ARRAY, gl_texture_array);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, width, heigth, depth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

void GPUTexture2DArray::set_as_rgb8(uint width, uint heigth, unsigned char* data) {
	set_as_rgb8(width, heigth, 1, data);
}

void GPUTexture2DArray::set_as_rgb8(uint width, uint heigth, uint depth, unsigned char* data) {
	glBindTexture(GL_TEXTURE_2D_ARRAY, gl_texture_array);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, heigth, depth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

uint GPUTexture2DArray::get_gl_type() const {
	return GL_TEXTURE_2D_ARRAY;
}

GPUCubemapTexture::GPUCubemapTexture() {
	glGenTextures(1, &gl_cubemap);
}

GL_ID GPUCubemapTexture::get_gl_id() const {
	return gl_cubemap;
}

void GPUCubemapTexture::set_as_depth(uint width, uint heigth, unsigned char* data) {
}

void GPUCubemapTexture::set_as_rgb8(uint width, uint heigth, unsigned char* data) {
}

void GPUCubemapTexture::set_as_rgb8(uint width, uint heigth, vector<unsigned char*> data) {
	use_texture();
	for (size_t i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, data[i]);
	}
}

uint GPUCubemapTexture::get_gl_type() const {
	return GL_TEXTURE_CUBE_MAP;
}

void GPUTexture::activate(uint id) {
	glActiveTexture(GL_TEXTURE0 + id);
	use_texture();
}

void GPUTexture::use_texture() {
	glBindTexture(get_gl_type(), get_gl_id());
}

void GPUTexture::set_as_depth_stencil(uint width, uint heigth, unsigned char* data) {
	use_texture();
	glTexImage2D(get_gl_type(), 0, GL_DEPTH24_STENCIL8, width, heigth, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
}

void GPUTexture::set_wrap(TextureWrap wrap) {
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

void GPUTexture::set_filter(TextureFilter filter) {
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

void GPUTexture::set_border_color(vec4 color) {
	use_texture();
	glTexParameterfv(get_gl_type(), GL_TEXTURE_BORDER_COLOR, &color.x);
}

void GPUPbrMaterial::update_internals() {
	auto shader = get_shader();
	shader->set_vec4("albedoColor", albedo);
	shader->set_vec4("emissiveColor", emissive);
	shader->set_float("metallicValue", metallic);
	shader->set_float("roughnessValue", roughness);
	shader->set_float("aoValue", ambient_occlusion);
}
