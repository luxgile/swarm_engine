#include "console_window.h"
#include <imgui.h>
#include <print>
#include "../../src/logging.h"

CConsoleWindow::CConsoleWindow() {
	title = "Console";

	//std::setbuf(stdout, &buffer);
	//old = std::cout.rdbuf(buffer.rdbuf());
	Console::log_info("Custom calling here");
}

//CConsoleWindow::~CConsoleWindow() {
//	std::cout.rdbuf(old);
//}

void CConsoleWindow::on_draw() {
	ImGui::TextUnformatted(Console::get_ouput_stream()->str().c_str());
}
