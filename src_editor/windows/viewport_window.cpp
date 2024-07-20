#include "viewport_window.h"
#include <imgui.h>
#include "../../src/core.h"
#include "../../src/rendering/render_plugin.h"
#include <print>

ImVec2 CViewportWindow::get_viewport_size(ImVec2 window_size) {
	if (abs(viewport_aspect) < 0.001) return window_size;

	float window_aspect = window_size.x / window_size.y;
	if (window_aspect > viewport_aspect) {
		return ImVec2(window_size.y * viewport_aspect, window_size.y);
	}
	else {
		return ImVec2(window_size.x, window_size.x / viewport_aspect);
	}
}

CViewportWindow::CViewportWindow() {
	title = "Viewport";
}

void CViewportWindow::pre_draw() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
}

void CViewportWindow::on_draw() {
	ImGui::PopStyleVar();

	auto app_ecs = App::get_main_world()->get_ecs();
	auto app_render_world = app_ecs->get<CRenderWorld>();

	if (!app_render_world || !app_render_world->world->vp) ImGui::Text("App ecs world is missing a RenderWorld.");
	else {
		auto wnd_size = ImGui::GetWindowSize();
		auto av_size = ImGui::GetContentRegionAvail();
		auto vp = app_render_world->world->vp.value();
		auto size = get_viewport_size(av_size);
		vp->set_size({ size.x, size.y });

		float posX = (wnd_size.x - size.x) * 0.5f;
		float posY = (wnd_size.y - size.y) * 0.5f;

		if (posX < 0.0f) posX = 0.0f;
		if (posY < 0.0f) posY = 0.0f;

		ImGui::SetCursorPos(ImVec2(posX, posY));
		ImGui::Image((void*)vp->get_color_ouput().value()->get_gl_id(), size, { 0, 1 }, { 1, 0 });
	}
}
