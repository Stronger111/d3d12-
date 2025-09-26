/**
 * @file shader_loader.h
 * @author Travis Vroman (travis@kohiengine.com)
 * @brief A resource loader that handles kshader config resources.
 * @version 1.0
 * @date 2022-02-28
 * 
 * @copyright Kohi Game Engine is Copyright (c) Travis Vroman 2021-2022
 * 
 */

#pragma once

#include "systems/resource_system.h"

/**
 * @brief Creates and returns a kshader resource loader.
 * 
 * @return The newly created resource loader.
 */
resource_loader shader_resource_loader_create(void);