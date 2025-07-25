#pragma once

#include "math/geometry.h"
#include "renderer/renderer_types.h"


typedef struct geometry_system_config {
    // Max number of geometries that can be loaded at once.
    // NOTE: Should be significantly greater than the number of static meshes because
    // the there can and will be more than one of these per mesh.
    // Take other systems into account as well
    u32 max_geometry_count;
} geometry_system_config;


#define DEFAULT_GEOMETRY_NAME "default"

/**
 * @brief Initializes the geometry system.
 * Should be called twice; once to get the memory requirement (passing state=0), and a second
 * time passing an allocated block of memory to actually initialize the system.
 *
 * @param memory_requirement A pointer to hold the memory requirement as it is calculated.
 * @param state A block of memory to hold the state or, if gathering the memory requirement, 0.
 * @param config The configuration (geometry_system_config) for this system.
 * @return True on success; otherwise false.
 */
b8 geometry_system_initialize(u64* memory_requirement, void* state, void* config);
void geometry_system_shutdown(void* state);

/**
 * @brief Acquires an existing geometry by id.
 *
 * @param id The geometry identifier to acquire by.
 * @return A pointer to the acquired geometry or nullptr if failed.
 */
geometry* geometry_system_acquire_by_id(u32 id);

/**
 * @brief Registers and acquires a new geometry using the given config.
 *
 * @param config The geometry configuration.
 * @param auto_release Indicates if the acquired geometry should be unloaded when its reference count reaches 0.
 * @return A pointer to the acquired geometry or nullptr if failed.
 */
KDEPRECATED("The geometry system acquire function will be removed in a future pass. Upload directly to renderbuffers instead.")
KAPI geometry* geometry_system_acquire_from_config(geometry_config config, b8 auto_release);

/**
 * @brief Frees resources held by the provided configuration.
 *
 * @param config A pointer to the configuration to be disposed.
 */
KAPI void geometry_system_config_dispose(geometry_config* config);

/**
 * @brief Releases a reference to the provided geometry.
 *
 * @param geometry The geometry to be released.
 */
void geometry_system_release(geometry* geometry);

/**
 * @brief Obtains a pointer to the default geometry.
 *
 * @return A pointer to the default geometry.
 */
geometry* geometry_system_get_default(void);

/**
 * @brief Obtains a pointer to the default geometry.
 *
 * @return A pointer to the default geometry.
 */
geometry* geometry_system_get_default_2d(void);

/**
 * @brief Generates configuration for plane geometries given the provided parameters.
 * NOTE: vertex and index arrays are dynamically allocated and should be freed upon object disposal.
 * Thus, this should not be considered production code.
 *
 * @param width The overall width of the plane. Must be non-zero.
 * @param height The overall height of the plane. Must be non-zero.
 * @param x_segment_count The number of segments along the x-axis in the plane. Must be non-zero.
 * @param y_segment_count The number of segments along the y-axis in the plane. Must be non-zero.
 * @param tile_x The number of times the texture should tile across the plane on the x-axis. Must be non-zero.
 * @param tile_y The number of times the texture should tile across the plane on the y-axis. Must be non-zero.
 * @param name The name of the generated geometry.
 * @param material_name The name of the material to be used.
 * @return A geometry configuration which can then be fed into geometry_system_acquire_from_config().
 */
geometry_config geometry_system_generate_plane_config(f32 width, f32 height, u32 x_segment_count, u32 y_segment_count, f32 tile_x, f32 tile_y, const char* name, const char* material_name);

/**
 * @brief
 *
 * @param width
 * @param height
 * @param depth
 * @param tile_x
 * @param tile_y
 * @param name
 * @param material_name
 * @return geometry_config
 */
KAPI geometry_config geometry_system_generate_cube_config(f32 width, f32 height, f32 depth, f32 tile_x, f32 tile_y, const char* name, const char* material_name);