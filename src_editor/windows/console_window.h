#pragma once
#include "editor_window.h"
#include <iostream>
#include <sstream>
#include <string>
#include <imgui.h>
#include "../../src/logging.h"

struct CConsoleWindow : public CEditorWindow {
private:
	int selected_id = -1;

	bool keep_at_bottom = false;

	ImU32 get_color_from_log(ConsoleLog* log);

public:
	CConsoleWindow();
	//~CConsoleWindow();
	virtual void on_draw() override;
};
