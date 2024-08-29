#pragma once

#include "defines.h"
#include "systems/font_system.h"
#include "renderer/renderer_types.inl"

struct game;

typedef struct application_config {
    i16 start_pos_x;
    i16 start_pos_Y;

    i16 start_width;
    i16 start_height;

    char* name;
    /** @brief Configuration for the font system. */
    font_system_config font_config;

    /** @brief A darray of render view configurations. */
   render_view_config* render_views;
} application_config;

KAPI b8 application_create(struct game* game_inst);

KAPI b8 application_run();

void application_get_framebuffer_size(u32* width, u32* height);
