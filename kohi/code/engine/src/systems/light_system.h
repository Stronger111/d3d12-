/**
 * @file light_system.h
 * @author Travis Vroman (travis@kohiengine.com)
 * @brief This file contains the implementation of the light system, which
 * manages all lighting objects within the engine.
 * @version 1.0
 * @date 2023-03-02
 *
 * @copyright Kohi Game Engine is Copyright (c) Travis Vroman 2021-2023
 *
 */
#pragma once

#include "defines.h"

#include "math/math_types.h"

typedef struct directional_light_data {
    /** @brief The light colour. */
    vec4 colour;
    /** @brief The direction of the light. The w component is ignored.*/
    vec4 direction;
} directional_light_data;
/**
 * @brief A directional light, typically used to emulate sun/moon light.
 */
typedef struct directional_light {
    /** @brief The name of the directional light. */
    char* name;
    /** @brief The directional light shader data. */
    directional_light_data data;
} directional_light;

typedef struct point_light_data {
    vec4 colour;
    vec4 position;  // ignore w
    // Usually 1, make sure denominator never gets smaller than 1
    f32 constant_f;
    // Reduces light intensity linearly
    f32 linear;
    // Makes the light fall off slower at longer distances.
    f32 quadratic;
    f32 padding;
} point_light_data;

/**
 * @brief A point light, the most common light source, which radiates out from the
 * given position.
 */
typedef struct point_light {
    /** @brief The name of the light. */
    char* name;
    /** @brief The shader data for the point light. */
    point_light_data data;
} point_light;

b8 light_system_initialize(u64* memory_requirement, void* memory, void* config);
void light_system_shutdown(void* state);

/**
 * @brief Attempts to add a directional light to the system. Only one may be present
 * at once, and is overwritten when one is passed here.
 *
 * @param light A pointer to the light to be added.
 * @return True on success; otherwise false.
 */
KAPI b8 light_system_directional_add(directional_light* light);
KAPI b8 light_system_point_add(point_light* light);

KAPI b8 light_system_directional_remove(directional_light* light);
KAPI b8 light_system_point_remove(point_light* light);

KAPI directional_light* light_system_directional_light_get(void);

KAPI u32 light_system_point_light_count(void);
KAPI b8 light_system_point_lights_get(point_light* out_light);