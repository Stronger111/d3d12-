#pragma once

#include "defines.h"

#include "resources/resource_types.h"

#define DEFAULT_MATERIAL_NAME "default"
/** @brief The name of the default UI material. */
#define DEFAULT_UI_MATERIAL_NAME "default_ui"

/** @brief The name of the default PBR material. */
#define DEFAULT_PBR_MATERIAL_NAME "default_pbr"

/** @brief The name of the default terrain material. */
#define DEFAULT_TERRAIN_MATERIAL_NAME "default_terrain"

typedef struct material_system_config {
    u32 max_material_count;
} material_system_config;

struct frame_data;

/**
 * @brief Initializes the material system.
 * Should be called twice; once to get the memory requirement (passing state=0), and a second
 * time passing an allocated block of memory to actually initialize the system.
 *
 * @param memory_requirement A pointer to hold the memory requirement as it is calculated.
 * @param state A block of memory to hold the state or, if gathering the memory requirement, 0.
 * @param config The configuration (material_system_config) for this system.
 * @return True on success; otherwise false.
 */
b8 material_system_initialize(u64* memory_requirement, void* state, void* config);
void material_system_shutdown(void* state);

KAPI material* material_system_acquire(const char* name);
/**
 * @brief Attempts to acquire a terrain material with the given name. If it has not yet been
 * loaded, this triggers it to be loaded from using the provided standard material names. If
 * the material is not able to be loaded, a pointer to the default terrain material is returned.
 * If the material _is_ found and loaded, its reference counter is incremented.
 *
 * @param name The name of the terrain material to find.
 * @param material_count The number of standard source material names.
 * @param material_names The names of the source materials to be used.
 * @return A pointer to the loaded terrain material. Can be a pointer to the defualt terrain material if not found.
 */
KAPI material* material_system_acquire_terrain_material(const char* material_name, u32 material_count, const char** material_names, b8 auto_release);

KAPI material* material_system_acquire_from_config(material_config* config);
KAPI void material_system_release(const char* name);

KAPI material* material_system_get_default(void);

/**
 * @brief Gets a pointer to the default PBR  material. Does not reference count.
 */
KAPI material* material_system_get_default_pbr(void);

/**
 * @brief Gets a pointer to the default terrain material. Does not reference count.
 */
KAPI material* material_system_get_default_terrain(void);

/**
 * @brief Dumps all of the registered materials and their reference counts/handles.
 */
KAPI void material_system_dump(void);