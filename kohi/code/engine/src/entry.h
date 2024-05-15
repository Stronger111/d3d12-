#include "core/application.h"
#include "core/logger.h"
#include "game_types.h"

//Externally -defiened function to create a game
extern b8 create_game(game* out_game);

int main(void)
{
   //application configuration
   application_config config;
   config.start_pos_x=100;
   config.start_pos_Y=100;
   config.start_width=1280;
   config.start_height=720;
   config.name="Kohi Engine Test";
   application_create(&config);
   application_run();
   return 0;
}