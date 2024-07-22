#pragma once

#include "editor_window.h"
#include "../../src/world.h"
#include "../../src/core.h"
#include "../../src/flecs/flecs.h"
#include "../../src/flecs/flecs.h"
#include <imgui.h>

class CWorldWindow : public CEditorWindow {
	Option<World*> selected_world = None;

	void draw_entity(flecs::entity e);

public:
	CWorldWindow();
	virtual void on_draw() override;
};
