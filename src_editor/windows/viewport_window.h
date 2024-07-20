#pragma once
#include "editor_window.h"
#include <imgui.h>

struct CViewportWindow : public CEditorWindow {
private:
	float viewport_aspect = 16.0 / 9.0;

	ImVec2 get_viewport_size(ImVec2 window_size);

public:
	CViewportWindow();
	virtual void pre_draw() override;
	virtual void on_draw() override;
};