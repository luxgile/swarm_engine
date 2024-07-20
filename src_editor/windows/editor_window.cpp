#include "editor_window.h"
#include <imgui.h>

void CEditorWindow::draw_window() {
	if(!visible) return;
	pre_draw();
	ImGui::Begin(title.c_str());
	on_draw();
	ImGui::End();
}
