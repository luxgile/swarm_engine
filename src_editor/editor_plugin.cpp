#include "editor_plugin.h"
#include <imgui.h>
#include "../src/core.h"
#include "../src/rendering/render_plugin.h"
#include "../src/flecs_helpers.h"
#include "windows/editor_window.h";
#include "windows/viewport_window.h";
#include <print>
#include "windows/console_window.h"


struct Unit {};
struct CombatUnit : Unit {};
struct MeleeUnit : CombatUnit {};
struct RangedUnit : CombatUnit {};

struct Warrior : MeleeUnit {};
struct Wizard : RangedUnit {};
struct Marksman : RangedUnit {};
struct Builder : Unit {};

Result<void, PluginError> EditorPlugin::setup_plugin(World* world) {
	auto editor_ecs = world->get_ecs();

	auto app_ecs = App::get_main_world()->get_ecs();
	auto app_render_world = app_ecs->get<CRenderWorld>();

	editor_ecs->system("Main editor docking space")
		.iter([](flecs::iter& it) {
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

	editor_ecs->entity("Viewport").add<CViewportWindow>();
	editor_ecs->entity("Console").add<CConsoleWindow>();

	editor_ecs->system<CViewportWindow>("Draw Viewport")
		.each([](flecs::entity e, CViewportWindow& wnd) {
		wnd.draw_window();
	});
	editor_ecs->system<CConsoleWindow>("Draw Console")
		.each([](flecs::entity e, CConsoleWindow& wnd) {
		wnd.draw_window();
	});

	//editor_ecs->system("Game Viewport").iter([app_render_world](flecs::iter& it) {
	//	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });

	//	ImGui::Begin("Viewport");

	//	ImGui::PopStyleVar();
	//	if (!app_render_world || !app_render_world->world->vp) ImGui::Text("App ecs world is missing a RenderWorld.");
	//	else {
	//		auto vp = app_render_world->world->vp.value();
	//		auto size = ImGui::GetContentRegionAvail();
	//		vp->set_size({ size.x, size.y });
	//		ImGui::Image((void*)vp->get_color_ouput().value()->get_gl_id(), size, { 0, 1 }, { 1, 0 });
	//	}
	//	ImGui::End();
	//});

	return Result<void, PluginError>();
}
