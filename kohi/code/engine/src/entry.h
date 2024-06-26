
#pragma once
#include "core/application.h"
#include "core/logger.h"
#include "core/kmemory.h"
#include "game_types.h"

//Externally -defiened function to create a game
extern b8 create_game(game* out_game);

int main(void)
{
    //Request the game instance from the application
    game game_inst;
    if(!create_game(&game_inst))
    {
        KFATAL("Could not create game!");
        return -1;
    }
    //Ensure the function pointer exist
    if(!game_inst.render||!game_inst.update||!game_inst.initialize||!game_inst.on_resize)
    {
        KFATAL("The games function pointers must be assigned!");
        return -2;
    }

    if(!application_create(&game_inst)) 
    {
       KINFO("Application filed to create!\n");
       return 1;
    }
    //Begin the game loop.
    if(!application_run())
    {
       KINFO("Application did not shutdown gracefully\n");
       return 2;
    }
   return 0;
}