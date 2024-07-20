#pragma once
#include "../src/core.h"

struct CVisible {
	bool value;
};

struct CUiWindow {
	std::string title;
	bool minimized;
};

struct CImGuiDrawCommand {
	std::function<void(void)> draw_command;
};


class EditorModule : public AplicationModule {
	World* editor_world;
	RenderWorld* editor_render_world;

public:
	void setup() override;
	void cleanup() override;

	//template<typename T>
	//void add_window(T* wnd) {
	//	static_assert(std::is_base_of<EditorWindow, T>(), "Error: T must inherit EditorWindow");
	//	windows.push_back(wnd);
	//}
};
