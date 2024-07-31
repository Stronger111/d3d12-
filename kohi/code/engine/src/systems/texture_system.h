#pragma once

#include "renderer/renderer_types.inl"

typedef struct texture_system_config
{
    u32 max_texture_count;
}texture_system_config;

#define DEFAULT_TEXTURE_NAME "default"

/** @brief The default diffuse texture name. */
#define DEFAULT_DIFFUSE_TEXTURE_NAME "default_DIFF"

/** @brief The default specular texture name. */
#define DEFAULT_SPECULAR_TEXTURE_NAME "default_SPEC"

/** @brief The default normal texture name. */
#define DEFAULT_NORMAL_TEXTURE_NAME "default_NORM"

b8 texture_system_initialize(u64* memory_requirement, void* state, texture_system_config config);
void texture_system_shutdown(void* state);

texture* texture_system_acquire(const char* name,b8 auto_release);
void texture_system_release(const char* name);

texture* texture_system_get_default_texture();

/**
 * @brief Gets a pointer to the default diffuse texture. No reference counting is 
 * done for default textures.
 */
texture* texture_system_get_default_diffuse_texture();

/**
 * @brief Gets a pointer to the default specular texture. No reference counting is 
 * done for default textures.
 */
texture* texture_system_get_default_specular_texture();

/**
 * @brief Gets a pointer to the default normal texture. No reference counting is 
 * done for default textures.
 */
texture* texture_system_get_default_normal_texture();
