#pragma once
#include "editor_window.h"
#include <iostream>
#include <sstream>
#include <string>

struct CConsoleWindow : public CEditorWindow {
private:


public:
	CConsoleWindow();
	//~CConsoleWindow();
	virtual void on_draw() override;
};
