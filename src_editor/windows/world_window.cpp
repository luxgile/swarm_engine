#include "world_window.h"

static int TREE_FLAGS = ImGuiTreeNodeFlags_SpanAllColumns;
static int TREE_LEAF_FLAGS = ImGuiTreeNodeFlags_SpanAllColumns
| ImGuiTreeNodeFlags_Leaf
| ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;

void CWorldWindow::draw_entity(CSelectedEntity* selected_e, flecs::entity e) {
	if (e.has(flecs::System) || e.has<flecs::Type>() || e.has(flecs::Module)) return;
	//TODO: Find out a way of doing this better.
	bool has_childen = false;
	e.children([&has_childen](flecs::entity e) {
		has_childen = true;
		return;
	});

	if (has_childen) {
		auto flags = TREE_FLAGS;
		if (e.id() == selected_e->entity) flags |= ImGuiTreeNodeFlags_Selected;
		bool open = ImGui::TreeNodeEx(e.name().c_str(), flags);
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) selected_e->entity = e.id();
		if (open) {
			e.children([this, selected_e](flecs::entity e) {
				draw_entity(selected_e, e);
			});
			ImGui::TreePop();
		}
	}
	else {
		auto flags = TREE_LEAF_FLAGS;
		if (e.id() == selected_e->entity) flags |= ImGuiTreeNodeFlags_Selected;
		ImGui::TreeNodeEx(e.name().c_str(), flags);
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) selected_e->entity = e.id();
	}
}

CWorldWindow::CWorldWindow() {
	title = "World View";
}

void CWorldWindow::on_draw() {
	auto& selected_world = editor_world->get_ecs()->ensure<CSelectedWorld>();
	if(!selected_world.world) return;
	auto worlds = App::get_worlds();
	if (ImGui::BeginCombo("Selected world", selected_world.world->get_name().c_str())) {

		for (auto world : worlds) {
			if (ImGui::Selectable(world->get_name().c_str(), selected_world.world == world)) {
				selected_world.world = world;
			}
		}

		ImGui::EndCombo();
	}

	if (!selected_world.world) {
		ImGui::Text("No world selected.");
		return;
	}

	auto ecs = selected_world.world->get_ecs();
	ecs->children([this](flecs::entity e) {
		if (e.has(flecs::System) || e.has<flecs::Type>() || e.has(flecs::Module)) return;
		auto& selected_e = editor_world->get_ecs()->ensure<CSelectedEntity>();
		draw_entity(&selected_e, e);
	});
}
