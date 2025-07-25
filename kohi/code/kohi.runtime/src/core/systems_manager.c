/*
#include "systems_manager.h"
#include "containers/darray.h"
#include "logger.h"

// Systems
#include "core/console.h"
#include "core/engine.h"
#include "core/event.h"
#include "core/input.h"
#include "kmemory.h"
#include "core/kvar.h"
#include "platform/platform.h"
#include "renderer/renderer_frontend.h"
#include "systems/audio_system.h"
#include "systems/camera_system.h"
#include "systems/geometry_system.h"
#include "systems/job_system.h"
#include "systems/light_system.h"
#include "systems/material_system.h"
#include "systems/resource_system.h"
#include "systems/shader_system.h"
#include "systems/texture_system.h"
#include "systems/timeline_system.h"

// Version reporting
#include "kohi.runtime_version.h"
#include "systems/xform_system.h"

static b8 register_known_systems_pre_boot(systems_manager_state* state, application_config* app_config);
static b8 register_known_systems_post_boot(systems_manager_state* state, application_config* app_config);
static void shutdown_known_systems(systems_manager_state* state);
static void shutdown_extension_systems(systems_manager_state* state);
static void shutdown_user_systems(systems_manager_state* state);

// TODO: Find a way to have this not be static.
static systems_manager_state* g_state;

b8 systems_manager_initialize(systems_manager_state* state, application_config* app_config) {
    // 创建一个线性分配器给所有的系统 64M
    linear_allocator_create(MEBIBYTES(64), 0, &state->systems_allocator);
    g_state = state;
    // 注册已知系统
    return register_known_systems_pre_boot(state, app_config);
}

b8 systems_manager_post_boot_initialize(systems_manager_state* state, application_config* app_config) {
    return register_known_systems_post_boot(state, app_config);
}

void systems_manager_shutdown(systems_manager_state* state) {
    shutdown_user_systems(state);
    shutdown_extension_systems(state);
    shutdown_known_systems(state);
}

b8 systems_manager_update(systems_manager_state* state, struct frame_data* p_frame_data) {
    for (u32 i = 0; i < K_SYSTEM_TYPE_MAX_COUNT; ++i) {
        k_system* s = &state->systems[i];
        if (s->update) {
            if (!s->update(s->state, p_frame_data)) {
                KERROR("System update failed for type:%i", i);
            }
        }
    }
    return true;
}

void systems_manager_renderer_frame_prepare(systems_manager_state* state, const struct frame_data* p_frame_data) {
    for (u32 i = 0; i < K_SYSTEM_TYPE_MAX_COUNT; ++i) {
        k_system* s = &state->systems[i];
        if (s->render_prepare_frame) {
            s->render_prepare_frame(s->state, p_frame_data);
        }
    }
}

b8 systems_manager_register(systems_manager_state* state, u16 type, PFN_system_initialize initialize, PFN_system_shutdown shutdown, PFN_system_update update, PFN_system_render_prepare_frame prepare_frame, void* config) {
    k_system* sys = &state->systems[type];
    // 调用初始化,开辟内存,调用初始化开始 w 已开辟内存块
    if (initialize) {
        sys->initialize = initialize;
        if (!sys->initialize(&sys->state_size, 0, config)) {
            KERROR("Failed to register system - initialize call failed.");
            return false;
        }

        sys->state = linear_allocator_allocate(&state->systems_allocator, sys->state_size);

        if (!sys->initialize(&sys->state_size, sys->state, config)) {
            KERROR("Failed to register system - initialize call failed.");
            kzero_memory(sys, sizeof(k_system));
            return false;
        }
    } else {
        if (type != K_SYSTEM_TYPE_MEMORY) {
            KERROR("Initialize is required for types except K_SYSTEM_TYPE_MEMORY.");
            return false;
        }
    }

    sys->shutdown = shutdown;
    sys->update = update;
    sys->render_prepare_frame = prepare_frame;
    return true;
}

void* systems_manager_get_state(u16 type) {
    if (g_state) {
        return g_state->systems[type].state;
    }

    return 0;
}

static b8 register_known_systems_pre_boot(systems_manager_state* state, application_config* app_config) {
    // Memory
    //     if (!systems_manager_register(state, K_SYSTEM_TYPE_MEMORY, 0, memory_system_shutdown, 0, 0, 0)) {
    //         KERROR("Failed to register memory system.");
    //         return false;
    //     }

    //     // Console
    //     if (!systems_manager_register(state, K_SYSTEM_TYPE_CONSOLE, console_initialize, console_shutdown, 0, 0, 0)) {
    //         KERROR("Failed to register console system.");
    //         return false;
    //     }

    //     // KVars
    //     if (!systems_manager_register(state, K_SYSTEM_TYPE_KVAR, kvar_initialize, kvar_shutdown, 0, 0, 0)) {
    //         KERROR("Failed to register KVar system.");
    //         return false;
    //     }

    //     // Events
    //     if (!systems_manager_register(state, K_SYSTEM_TYPE_EVENT, event_system_initialize, event_system_shutdown, 0, 0, 0)) {
    //         KERROR("Failed to register event system.");
    //         return false;
    //     }

    //     // Logging
    //     if (!systems_manager_register(state, K_SYSTEM_TYPE_LOGGING, logging_initialize, logging_shutdown, 0, 0, 0)) {
    //         KERROR("Failed to register logging system.");
    //         return false;
    //     }

    // // Report engine version
    // #if KRELEASE
    //     const char* build_type = "Release";
    // #else
    //     const char* build_type = "Debug";
    // #endif
    //     KINFO("Kohi Engine v. %s (%s)", KVERSION, build_type);

    //     // Input
    //     if (!systems_manager_register(state, K_SYSTEM_TYPE_INPUT, input_system_initialize, input_system_shutdown, 0, 0, 0)) {
    //         KERROR("Failed to register input system.");
    //         return false;
    //     }

    //     // Platform
    //     platform_system_config plat_config = {0};
    //     plat_config.application_name = app_config->name;
    //     plat_config.x = app_config->start_pos_x;
    //     plat_config.y = app_config->start_pos_y;
    //     plat_config.width = app_config->start_width;
    //     plat_config.height = app_config->start_height;
    //     if (!systems_manager_register(state, K_SYSTEM_TYPE_PLATFORM, platform_system_startup, platform_system_shutdown, 0, 0, &plat_config)) {
    //         KERROR("Failed to register platform system.");
    //         return false;
    //     }

    return true;
}

static void shutdown_extension_systems(systems_manager_state* state) {
    // NOTE:Anything between 127-254 is extension systems.
    for (u32 i = K_SYSTEM_TYPE_KNOWN_MAX; i < K_SYSTEM_TYPE_EXT_MAX; ++i) {
        if (state->systems[i].shutdown) {
            state->systems[i].shutdown(state->systems[i].state);
        }
    }
}

static void shutdown_user_systems(systems_manager_state* state) {
    // NOTE:Anything beyond this  is in user space.
    for (u32 i = K_SYSTEM_TYPE_USER_MAX; i < K_SYSTEM_TYPE_MAX; ++i) {
        if (state->systems[i].shutdown) {
            state->systems[i].shutdown(state->systems[i].state);
        }
    }
}

void shutdown_known_systems(systems_manager_state* state) {
    state->systems[K_SYSTEM_TYPE_CAMERA].shutdown(state->systems[K_SYSTEM_TYPE_CAMERA].state);
    state->systems[K_SYSTEM_TYPE_FONT].shutdown(state->systems[K_SYSTEM_TYPE_FONT].state);

    state->systems[K_SYSTEM_TYPE_GEOMETRY].shutdown(state->systems[K_SYSTEM_TYPE_GEOMETRY].state);
    state->systems[K_SYSTEM_TYPE_MATERIAL].shutdown(state->systems[K_SYSTEM_TYPE_MATERIAL].state);
    state->systems[K_SYSTEM_TYPE_TEXTURE].shutdown(state->systems[K_SYSTEM_TYPE_TEXTURE].state);

    state->systems[K_SYSTEM_TYPE_JOB].shutdown(state->systems[K_SYSTEM_TYPE_JOB].state);
    state->systems[K_SYSTEM_TYPE_SHADER].shutdown(state->systems[K_SYSTEM_TYPE_SHADER].state);
    state->systems[K_SYSTEM_TYPE_RENDERER].shutdown(state->systems[K_SYSTEM_TYPE_RENDERER].state);

    state->systems[K_SYSTEM_TYPE_RESOURCE].shutdown(state->systems[K_SYSTEM_TYPE_RESOURCE].state);
    state->systems[K_SYSTEM_TYPE_PLATFORM].shutdown(state->systems[K_SYSTEM_TYPE_PLATFORM].state);
    state->systems[K_SYSTEM_TYPE_INPUT].shutdown(state->systems[K_SYSTEM_TYPE_INPUT].state);
    state->systems[K_SYSTEM_TYPE_LOGGING].shutdown(state->systems[K_SYSTEM_TYPE_LOGGING].state);
    state->systems[K_SYSTEM_TYPE_EVENT].shutdown(state->systems[K_SYSTEM_TYPE_EVENT].state);
    state->systems[K_SYSTEM_TYPE_KVAR].shutdown(state->systems[K_SYSTEM_TYPE_KVAR].state);
    state->systems[K_SYSTEM_TYPE_CONSOLE].shutdown(state->systems[K_SYSTEM_TYPE_CONSOLE].state);

    state->systems[K_SYSTEM_TYPE_MEMORY].shutdown(state->systems[K_SYSTEM_TYPE_MEMORY].state);
}

static b8 register_known_systems_post_boot(systems_manager_state* state, application_config* app_config) {
    // Xform system.
    xform_system_config xform_sys_config = {0};
    xform_sys_config.initial_slot_count = 128;  // TODO: expose to app config.
    if (!systems_manager_register(state, K_SYSTEM_TYPE_XFORM, xform_system_initialize, xform_system_shutdown, xform_system_update, 0, &xform_sys_config)) {
        KERROR("Failed to register xform system.");
        return false;
    }

    // Texture system.
    texture_system_config texture_sys_config;
    texture_sys_config.max_texture_count = 65536;
    if (!systems_manager_register(state, K_SYSTEM_TYPE_TEXTURE, texture_system_initialize, texture_system_shutdown, 0, 0, &texture_sys_config)) {
        KERROR("Failed to register texture system.");
        return false;
    }

    // Font system.
    // if (!systems_manager_register(state, K_SYSTEM_TYPE_FONT, font_system_initialize, font_system_shutdown, 0, 0, &app_config->font_config)) {
    //     KERROR("Failed to register font system.");
    //     return false;
    // }

    // Camera
    camera_system_config camera_sys_config;
    camera_sys_config.max_camera_count = 61;
    if (!systems_manager_register(state, K_SYSTEM_TYPE_CAMERA, camera_system_initialize, camera_system_shutdown, 0, 0, &camera_sys_config)) {
        KERROR("Failed to register camera system.");
        return false;
    }

    // Material system.
    material_system_config material_sys_config;
    material_sys_config.max_material_count = 4096;
    if (!systems_manager_register(state, K_SYSTEM_TYPE_MATERIAL, material_system_initialize, material_system_shutdown, 0, 0, &material_sys_config)) {
        KERROR("Failed to register material system.");
        return false;
    }

    // Geometry system.
    geometry_system_config geometry_sys_config;
    geometry_sys_config.max_geometry_count = 4096;
    if (!systems_manager_register(state, K_SYSTEM_TYPE_GEOMETRY, geometry_system_initialize, geometry_system_shutdown, 0, 0, &geometry_sys_config)) {
        KERROR("Failed to register geometry system.");
        return false;
    }

    // Light system.
    if (!systems_manager_register(state, K_SYSTEM_TYPE_LIGHT, light_system_initialize, light_system_shutdown, 0, 0, 0)) {
        KERROR("Failed to register light system.");
        return false;
    }
    return true;
}
*/