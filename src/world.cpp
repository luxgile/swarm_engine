#include "world.h"

World::World() {
	ecs = new flecs::world();
}

void World::process_frame(float dt) {
	ecs->progress(dt);
}
