#include "editor_plugin.h"
#include <imgui.h>
#include "../src/core.h"
#include "../src/rendering/render_plugin.h"
#include "../src/flecs_helpers.h"
#include "windows/editor_window.h"
#include "windows/viewport_window.h"
#include "windows/world_window.h"
#include <print>
#include "windows/console_window.h"
#include "../src/logging.h"

Result<void, PluginError> EditorPlugin::setup_plugin(World* world) {
	auto editor_ecs = world->get_ecs();

	auto app_ecs = App::get_main_world()->get_ecs();
	auto app_render_world = app_ecs->get<CRenderWorld>();

	editor_ecs->system("Main editor docking space")
		.run([](flecs::iter& it) {
		auto wnd = App::get_render_backend()->get_main_window();
		auto wnd_size = wnd->get_size();
		auto wnd_flags = ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoBackground;

		ImGui::SetNextWindowPos({ 0, 0 });
		ImGui::SetNextWindowSize({ (float)wnd_size.x, (float)wnd_size.y });

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });

		ImGui::Begin("Main Editor Window", nullptr, wnd_flags);

		ImGui::PopStyleVar();
		ImGui::DockSpace(ImGui::GetID("Dockspace"), { 0, 0 });
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Exit")) {
					wnd->close();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		ImGui::End();
	});

	editor_ecs->component<CEditorWindow>().member<bool>("visible");
	editor_ecs->component<CViewportWindow>().is_a<CEditorWindow>();
	editor_ecs->component<CConsoleWindow>().is_a<CEditorWindow>();
	editor_ecs->component<CWorldWindow>().is_a<CEditorWindow>();

	editor_ecs->entity("Viewport").add<CViewportWindow>();
	editor_ecs->entity("Console").add<CConsoleWindow>();
	editor_ecs->entity("World View").add<CWorldWindow>();

	editor_ecs->system<CEditorWindow>("Draw Viewport")
		.each([](flecs::entity e, CEditorWindow& wnd) {
		auto result = wnd.draw_window();
		if (!result) Console::log_error(result.error().error.c_str());
	});

	return Result<void, PluginError>();
}
