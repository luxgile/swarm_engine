#pragma once

#include "editor_window.h"
#include "../../src/world.h"
#include "../../src/core.h"
#include "../../src/flecs/flecs.h"
#include "../../src/flecs/flecs.h"
#include "../editor_plugin.h"
#include <imgui.h>

class CWorldWindow : public CEditorWindow {
	void draw_entity(CSelectedEntity* selected_e, flecs::entity e);

public:
	CWorldWindow();
	virtual void on_draw() override;
};
