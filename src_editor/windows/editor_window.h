#pragma once
#include <string>

struct CEditorWindow {
	std::string title = "Title";
	bool visible = true;
	
	virtual void pre_draw() {}
	void draw_window();
	virtual void on_draw() {}
};
