#include "world.h"

World::World(std::string name) : name(name) {
	ecs = new flecs::world();
}

void World::process_frame(float dt) {
	ecs->progress(dt);
}

void World::toggle_flecs_rest(bool state) {
	if (state) {
		ecs->set<flecs::Rest>({});
		ecs->import<flecs::stats>();
	}
	else ecs->remove<flecs::Rest>();
}
