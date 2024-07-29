#include "entity_window.h"

CEntityWindow::CEntityWindow() {
	title = "Entity View";
}

void CEntityWindow::on_draw() {
	auto editor_ecs = editor_world->get_ecs();
	auto selected_ecs = editor_ecs->ensure<CSelectedWorld>().world->get_ecs();
	auto entity_id = editor_ecs->ensure<CSelectedEntity>().entity;

	auto entity = selected_ecs->get_alive(entity_id);
	if (!entity.is_valid()) return;
	
	ImGui::Text(entity.name());
}
