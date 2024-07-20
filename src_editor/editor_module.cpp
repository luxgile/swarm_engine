#include "editor_module.h"

#include "../src/imgui/imgui_plugin.h"
#include "editor_plugin.h"
#include <print>

void EditorModule::setup() {
	editor_world = App::create_world();
	editor_world->toggle_flecs_rest(true);

	editor_render_world = App::get_render_backend()->worlds.create();

	auto ecs = editor_world->get_ecs();

	auto rrender = editor_world->add_plugin<RenderPlugin>();
	if(!rrender) std::println("{}", rrender.error().error);

	auto rimgui = editor_world->add_plugin<ImGuiPlugin>();
	if(!rimgui) std::println("{}", rimgui.error().error);

	auto reditor = editor_world->add_plugin<EditorPlugin>();
	if(!reditor) std::println("{}", reditor.error().error);
}

void EditorModule::cleanup() {
}

