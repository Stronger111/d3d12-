#pragma once

#include "core/application.h"

typedef struct  game
{
    application_config app_config;
    //函数指针  
    b8 (*initialize)(struct game* game_list);
    //Function pointer to games update function
    b8 (*update)(struct game* game_list,f32 delta_time);

    b8 (*render)(struct game* game_list,f32 delta_time);
    
    void (*on_resize)(struct game* game_list,u32 width,u32 height);

    //Game specific game state created and managed by the game
    void* state;
 
}game;
