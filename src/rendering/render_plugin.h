#pragma once

#include "renderer.h"
#include "../plugin.h"
#include "../world.h"

struct CRenderWorld {
	RenderWorld* world;
};

class RenderPlugin : public Plugin {
public:
	Result<void, PluginError> setup_plugin(World* world) override;
};
