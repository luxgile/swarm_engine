#include "world_window.h"

CWorldWindow::CWorldWindow() {
	title = "World View";
}

void CWorldWindow::on_draw() {
	auto worlds = App::get_worlds();
	if (ImGui::BeginCombo("Selected world", selected_world ? selected_world.value()->get_name().c_str() : "None")) {
		if (ImGui::Selectable("None", !selected_world.has_value())) {
			selected_world = None;
		}

		for (auto world : worlds) {
			if (ImGui::Selectable(world->get_name().c_str(), selected_world ? selected_world.value() == world : false)) {
				selected_world = world;
			}
		}

		ImGui::EndCombo();
	}

	if (!selected_world) {
		ImGui::Text("No world selected.");
		return;
	}

	auto ecs = selected_world.value()->get_ecs();
	ecs->children([](flecs::entity e) {
		ImGui::Text(e.name().c_str());
	});
}
