#pragma once

#include <string>
#include "venum.h"
#include "world.h"

class World;

class PluginError {
public:
	std::string error;
};

class Plugin {
public:
	virtual Result<void, PluginError> setup_plugin(World* world) = 0;
};
