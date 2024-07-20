#include "render_plugin.h"

#include "../core.h"

Result<void, PluginError> RenderPlugin::setup_plugin(World* world) {
    auto ecs = world->get_ecs();

    auto render_world = App::get_render_backend()->worlds.create();
    render_world->vp = App::get_render_backend()->viewports.create();

    auto main_window = App::get_render_backend()->get_main_window();
    main_window->set_viewport(render_world->vp.value());

    ecs->set<CRenderWorld>({render_world});

    return Result<void, PluginError>();
}
