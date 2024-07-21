#include "imgui_plugin.h"

#include "../flecs_helpers.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

Result<void, PluginError> ImGuiPlugin::setup_plugin(World* world) {
	if (!world->has_plugin<RenderPlugin>()) return Error(PluginError{ .error = "Dependency with plugin RenderPlugin is missing" });

	auto ecs = world->get_ecs();

	reflect_component(ecs, CImGuiEnabled)
		reflect_var(bool, CImGuiEnabled::value);

	ecs->entity("ImGui").set<CImGuiEnabled>({ true });

	ecs->system<const CImGuiEnabled>("Start ImGui Frame")
		.kind(flecs::OnLoad)
		.each([](const CImGuiEnabled& enabled) {
		if (!enabled.value) return;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	});

	//ecs->system("Imgui Demo Window")
	//	.iter([](flecs::iter& it) {
	//	ImGui::ShowDemoWindow();
	//});

	ecs->system<const CImGuiEnabled>("End ImGui Frame")
		.kind(flecs::OnStore)
		.each([](flecs::entity e, const CImGuiEnabled& enabled) {
		if (!enabled.value) return;
		ImGui::Render();
		auto render_world = e.world().get_mut<CRenderWorld>();
		render_world->world->imgui_draw_cmd = ImGui::GetDrawData();
	});
}
