#include "editor_window.h"
#include <imgui.h>
#include <format>

Result<void, EditorWindowError> CEditorWindow::draw_window() {
	if (!visible) return Result<void, EditorWindowError>();

	try {
		pre_draw();
	}
	catch (const std::exception& error) {
		return Error(EditorWindowError{ std::format("'{}' predraw exception - {}", title, error.what()) });
	}

	ImGui::Begin(title.c_str());

	try {
		on_draw();
	}
	catch (const std::exception& error) {
		ImGui::End();
		return Error(EditorWindowError{ std::format("'{}' draw exception - {}", title, error.what()) });
	}

	ImGui::End();
}
