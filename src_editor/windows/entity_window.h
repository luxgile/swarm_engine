#pragma once

#include "editor_window.h"
#include "../editor_plugin.h"
#include "../../src/world.h"
#include "../../src/core.h"
#include "../../src/flecs/flecs.h"
#include "../../src/flecs/flecs.h"
#include <imgui.h>

class CEntityWindow : public CEditorWindow {
public:
	CEntityWindow();
	virtual void on_draw() override;
};
