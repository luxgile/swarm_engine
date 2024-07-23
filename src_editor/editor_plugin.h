#pragma once

#include "../src/plugin.h"
#include <imgui.h>

struct CSelectedEntity {
	flecs::entity_t entity;
};

class EditorPlugin : public Plugin {
public:
	Result<void, PluginError> setup_plugin(World* world) override;
};
