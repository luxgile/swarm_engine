#pragma once
#include "flecs/flecs.h"
#include "plugin.h"
#include <typeindex>
#include <typeinfo>
#include <vector>

class Plugin;
class PluginError;

class World;

class World {
	std::string name;
	flecs::world* ecs;

	std::vector<std::type_index> plugins_intalled;

public:
	World(std::string name = "");
	void process_frame(float dt);

	void toggle_flecs_rest(bool state);
	flecs::world* get_ecs() { return ecs; }

	const std::string get_name() const { return name; }
	void set_name(std::string name) { this->name = name; }

	template<typename T>
	bool has_plugin() {
		std::type_index type_id = typeid(T);
		return std::find(plugins_intalled.begin(), plugins_intalled.end(), type_id) != plugins_intalled.end();
	}

	template<typename T>
	Result<void, PluginError> add_plugin() {
		static_assert(std::is_base_of<Plugin, T>(), "T must be a plugin.");

		if (has_plugin<T>()) return Error(PluginError{ .error = "Error: Plugin already installed into world." });

		auto plugin = T();
		auto world = this;
		auto result = plugin.setup_plugin(world);
		if (!result) return result;

		plugins_intalled.push_back(typeid(T));
	}
};
