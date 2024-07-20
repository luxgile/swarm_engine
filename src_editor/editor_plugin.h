#pragma once

#include "../src/plugin.h"
#include <imgui.h>


class EditorPlugin : public Plugin {
	

public:
	Result<void, PluginError> setup_plugin(World* world) override;
};
