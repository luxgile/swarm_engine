#pragma once

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define reflect_component(ecs, type) ecs->component<type>()
#define reflect_var(type, name) .member<type>(STRINGIFY(name))
