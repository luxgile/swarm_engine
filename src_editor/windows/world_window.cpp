#include "world_window.h"

void CWorldWindow::draw_entity(flecs::entity e) {
	if (e.has(flecs::System) || e.has<flecs::Type>() || e.has(flecs::Module)) return;
	//TODO: Find out a way of doing this better.
	bool has_childen = false;
	e.children([&has_childen](flecs::entity e) {
		has_childen = true;
		return;
	});

	if (has_childen) {
		bool open = ImGui::TreeNodeEx(e.name().c_str(), ImGuiTreeNodeFlags_SpanAllColumns);
		if (open) {
			e.children([this](flecs::entity e) {
				draw_entity(e);
			});
			ImGui::TreePop();
		}
	}
	else {
		ImGui::TreeNodeEx(e.name().c_str(),
			ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_Leaf
			| ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
	}
}

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
	ecs->children([this](flecs::entity e) {
		if (e.has(flecs::System) || e.has<flecs::Type>() || e.has(flecs::Module)) return;
		draw_entity(e);
	});
}
