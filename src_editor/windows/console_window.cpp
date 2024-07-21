#include "console_window.h"
#include <print>

ImU32 CConsoleWindow::get_color_from_log(ConsoleLog* log) {
	switch (log->get_type()) {
	case LogType::LogVerbose:
		return ImGui::GetColorU32({ 0.1, 0.1, 0.1, 1 });
	case LogType::LogInfo:
		return ImGui::GetColorU32({ 0.1, 0.1, 0.3, 1 });
	case LogType::LogWarning:
		return ImGui::GetColorU32({ 0.4, 0.4, 0.0, 1 });
	case LogType::LogError:
		return ImGui::GetColorU32({ 0.4, 0.1, 0.1, 1 });
	case LogType::LogCritical:
		return ImGui::GetColorU32({ 0.3, 0.1, 0.3, 1 });
	}
}

CConsoleWindow::CConsoleWindow() {
	title = "Console";
}

void CConsoleWindow::on_draw() {
	auto logs = Console::get_logs();

	if (ImGui::SmallButton("Clear")) {
		Console::clear();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(keep_at_bottom);
	if (ImGui::SmallButton("Follow")) {
		keep_at_bottom = true;
	}
	ImGui::EndDisabled();

	ImGui::Separator();

	auto flags =
		ImGuiTableFlags_Resizable
		| ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY
		| ImGuiTableFlags_Borders
		| ImGuiTableFlags_SizingFixedFit;
	auto columns_base_flags = ImGuiTableColumnFlags_None;

	if (ImGui::BeginTable("console_table", 3, flags, { 0, 0 })) {
		ImGui::TableSetupColumn("Type", columns_base_flags);
		ImGui::TableSetupColumn("Log", columns_base_flags);
		ImGui::TableSetupScrollFreeze(0, 1);

		ImGui::TableHeadersRow();

		ImGuiListClipper clipper;
		clipper.Begin(logs.size());
		while (clipper.Step()) {
			for (size_t i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
				auto log = logs[i];

				ImGui::PushID(i);
				ImGui::TableNextRow();
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, get_color_from_log(&log));
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(std::to_string(log.get_type()).c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text(log.get_log().c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::Text(boost::stacktrace::to_string(log.get_stacktrace()[4]).c_str());
				ImGui::PopID();
			}
		}
		if (keep_at_bottom) {
			ImGui::SetScrollY(ImGui::GetScrollMaxY());
		}

		if (ImGui::GetIO().MouseWheel > 0.5f) {
			keep_at_bottom = false;
		}


		ImGui::EndTable();
	}
}
