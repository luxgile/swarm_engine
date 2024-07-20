#include "editor_plugin.h"
#include <imgui.h>
#include "../src/core.h"
#include "../src/rendering/render_plugin.h"

Result<void, PluginError> EditorPlugin::setup_plugin(World* world) {
	auto editor_ecs = world->get_ecs();

	auto app_ecs = App::get_main_world()->get_ecs();
	auto app_render_world = app_ecs->get<CRenderWorld>();

	editor_ecs->system("Game Viewport").iter([app_render_world](flecs::iter& it) {
		ImGui::Begin("Viewport");
		if (!app_render_world || !app_render_world->world->vp) ImGui::Text("App ecs world is missing a RenderWorld.");
		else {
			auto vp = app_render_world->world->vp.value();
			ImGui::Image((void*)vp->get_color_ouput().value()->get_gl_id(), { vp->get_size().x, vp->get_size().y }, {0, 1}, {1, 0});
		}
		ImGui::End();
	});

	return Result<void, PluginError>();
}
