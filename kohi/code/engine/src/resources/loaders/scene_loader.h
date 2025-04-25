/**
 * @file simple_scene_loader.h
 * @author Travis Vroman (travis@kohiengine.com)
 * @brief Loader for simple scene files.
 * @version 1.0
 * @date 2023-03-29
 * 
 * @copyright Kohi Game Engine is Copyright (c) Travis Vroman 2021-2024
 * 
 */
#pragma once

#include "systems/resource_system.h"

/**
 * @brief Creates and returns a scene resource loader.
 * 
 * @return The newly created resource loader.
 */
KAPI resource_loader scene_resource_loader_create(void);