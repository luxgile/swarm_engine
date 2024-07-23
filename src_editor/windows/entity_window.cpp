#include "entity_window.h"

CEntityWindow::CEntityWindow() {
	title = "Entity View";
}

void CEntityWindow::on_draw() {
	//auto ecs = editor_world->get_ecs();

	//auto entity_id = ecs->ensure<CSelectedEntity>().entity;
	//auto entity = ecs->get_alive(entity_id);
	//if (!entity.is_valid()) return;
	//
	//ImGui::Text(entity.name());
}
