#pragma once
#include "../../src/venum.h"
#include <string>
#include "../../src/world.h"

struct EditorWindowError {
	std::string error;
};

struct CEditorWindow {
	World* editor_world;
	std::string title = "Title";
	bool visible = true;

	CEditorWindow() = default;
	virtual void pre_draw() {}
	Result<void, EditorWindowError> draw_window();
	virtual void on_draw() {}
};
