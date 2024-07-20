#include "world.h"

World::World() {
	ecs = new flecs::world();
}

void World::process_frame(float dt) {
	ecs->progress(dt);
}

void World::toggle_flecs_rest(bool state) {
	if (state) ecs->set<flecs::Rest>({});
	else ecs->remove<flecs::Rest>();
}
