#pragma once
#include <imgui.h>
#include "../plugin.h"
#include "../rendering/renderer.h"
#include "../rendering/render_plugin.h"
#include <flecs.h>

struct CImGuiEnabled {
	bool value;
};

class ImGuiPlugin : public Plugin {
public:
	Result<void, PluginError> setup_plugin(World* world) override;
};
