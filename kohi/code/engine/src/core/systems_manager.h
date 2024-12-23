#pragma once

#include "defines.h"
#include "memory/linear_allocator.h"

struct frame_data;

typedef b8 (*PFN_system_initialize)(u64* memory_requirement, void* memory, void* config);
typedef void (*PFN_system_shutdown)(void* state);
typedef b8 (*PFN_system_update)(void* state,struct frame_data* p_frame_data);

typedef struct k_system {
    u64 state_size;
    void* state;
    PFN_system_initialize initialize;
    PFN_system_shutdown shutdown;
    /** @brief A function pointer for the system's update routine, called every frame. Optional. */
    PFN_system_update update;
} k_system;

#define K_SYSTEM_TYPE_MAX_COUNT 512

typedef enum k_system_type {
    K_SYSTEM_TYPE_MEMORY = 0,
    K_SYSTEM_TYPE_CONSOLE,
    K_SYSTEM_TYPE_KVAR,
    K_SYSTEM_TYPE_EVENT,
    K_SYSTEM_TYPE_LOGGING,
    K_SYSTEM_TYPE_INPUT,
    K_SYSTEM_TYPE_PLATFORM,
    K_SYSTEM_TYPE_RESOURCE,
    K_SYSTEM_TYPE_SHADER,
    K_SYSTEM_TYPE_JOB,
    K_SYSTEM_TYPE_TEXTURE,
    K_SYSTEM_TYPE_FONT,
    K_SYSTEM_TYPE_CAMERA,
    K_SYSTEM_TYPE_RENDERER,
    K_SYSTEM_TYPE_RENDER_VIEW,
    K_SYSTEM_TYPE_MATERIAL,
    K_SYSTEM_TYPE_GEOMETRY,
    K_SYSTEM_TYPE_LIGHT,

    // NOTE: Anything between 127-254 is extension space.
    K_SYSTEM_TYPE_KNOWN_MAX = 127,

    // NOTE: Anything beyond this is in user space.
    K_SYSTEM_TYPE_EXT_MAX = 255,

    // 用户空间
    K_SYSTEM_TYPE_USER_MAX = K_SYSTEM_TYPE_MAX_COUNT,
    // The max, including all user-space types.
    K_SYSTEM_TYPE_MAX = K_SYSTEM_TYPE_USER_MAX
} k_system_type;

typedef struct systems_manager_state {
    linear_allocator systems_allocator;
    k_system systems[K_SYSTEM_TYPE_MAX_COUNT];
} systems_manager_state;

struct application_config;

b8 systems_manager_initialize(systems_manager_state* state, struct application_config* app_config);
b8 systems_manager_post_boot_initialize(systems_manager_state* state, struct application_config* app_config);
void systems_manager_shutdown(systems_manager_state* state);
/**
 * @brief Calls update routines on all systems that opt in to the update.
 * Performed during the main engine loop.
 *
 * @param state A pointer to the systems manager state.
 * @param p_frame_data A constant pointer to the data for this frame.
 * @return b8 True on success; otherwise false.
 */
b8 systems_manager_update(systems_manager_state* state,struct frame_data* p_frame_data);

KAPI b8 systems_manager_register(systems_manager_state* state, u16 type, PFN_system_initialize initialize, PFN_system_shutdown shutdown, PFN_system_update update, void* config);

KAPI void* systems_manager_get_state(u16 type);