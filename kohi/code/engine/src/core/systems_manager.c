#include "systems_manager.h"

#include "core/logger.h"
#include "containers/darray.h"

// Systems
#include "core/kmemory.h"
#include "core/engine.h"
#include "core/console.h"
#include "core/kvar.h"
#include "core/event.h"
#include "core/input.h"
#include "platform/platform.h"
#include "systems/resource_system.h"
#include "systems/shader_system.h"
#include "renderer/renderer_frontend.h"
#include "systems/job_system.h"
#include "systems/texture_system.h"
#include "systems/camera_system.h"
#include "systems/render_view_system.h"
#include "systems/material_system.h"
#include "systems/geometry_system.h"

b8 systems_manager_initialize(systems_manager_state* state, application_config* app_config) {
    return false;
}