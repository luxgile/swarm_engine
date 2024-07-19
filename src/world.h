#pragma once
#include <flecs.h>

class World {
	flecs::world* ecs;

public:
	World();
	void process_frame(float dt);
};
