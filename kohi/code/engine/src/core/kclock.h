/**
 * @file kclock.h
 * @author Travis Vroman (travis@kohiengine.com)
 * @brief This file contains structures and functions for the 
 * engine's clock.
 * @version 1.0
 * @date 2022-01-10
 * 
 * @copyright Kohi Game Engine is Copyright (c) Travis Vroman 2021-2022
 * 
 */

 #pragma once
 #include "defines.h"

typedef struct kclock
{
   f64 start_time;
   f64 elapsed;
}kclock;

//Updates the provided clock. Should be called just before checking elapsed time.
//Has no effect on non-started clocks
KAPI void kclock_update(kclock* clock);

//Start the provided clock.Resets elapsed time
KAPI void kclock_start(kclock* clock);

//Stops the provided clock. Does not reset elapsed time
KAPI void kclock_stop(kclock* clock);
