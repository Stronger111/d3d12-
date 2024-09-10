#pragma once

#include "core/application.h"
#include "memory/linear_allocator.h"

struct render_packet;

typedef struct game_frame_data {
    // A darray of world geometries to be rendered this frame.
    geometry_render_data* world_geometries;
} game_frame_data;

typedef struct game {
    application_config app_config;
    /**
     * @brief Function pointer to the game's boot sequence. This should
     * fill out the application config with the game's specific requirements.
     * @param game_inst A pointer to the game instance.
     * @returns True on success; otherwise false.
     */
    b8 (*boot)(struct game* game_inst);
    // 函数指针
    b8 (*initialize)(struct game* game_list);
    // Function pointer to games update function
    b8 (*update)(struct game* game_list, f32 delta_time);

    /**
     * @brief Function pointer to game's render function.
     * @param game_inst A pointer to the game instance.
     * @param packet A pointer to the packet to be populated by the game.
     * @param delta_time The time in seconds since the last frame.
     * @returns True on success; otherwise false.
     * */
    b8 (*render)(struct game* game_list, struct render_packet* packet, f32 delta_time);

    void (*on_resize)(struct game* game_list, u32 width, u32 height);

    /**
     * @brief Shuts down the game, prompting release of resources.
     * @param game_inst A pointer to the game instance.
     */
    void (*shutdown)(struct game* game_inst);

    /** @brief The required size for the game state. */
    u64 state_memory_requirement;

    // Game specific game state created and managed by the game
    void* state;

    /// Application state
    void* application_state;
    /**
     * @brief An allocator used for allocations needing to be made every frame. Contents are wiped
     * at the beginning of the frame.
     */
    linear_allocator frame_allocator;

    /** @brief Data which is built up, used and discarded every frame. */
    game_frame_data frame_data;
} game;
