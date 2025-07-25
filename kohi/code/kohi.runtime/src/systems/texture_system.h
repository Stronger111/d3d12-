#pragma once

#include "renderer/renderer_types.h"

typedef struct texture_system_config {
    u32 max_texture_count;
} texture_system_config;

#define DEFAULT_TEXTURE_NAME "default"

/** @brief The default diffuse texture name. */
#define DEFAULT_DIFFUSE_TEXTURE_NAME "default_DIFF"

/** @brief The default specular texture name. */
#define DEFAULT_SPECULAR_TEXTURE_NAME "default_SPEC"

/** @brief The default normal texture name. */
#define DEFAULT_NORMAL_TEXTURE_NAME "default_NORM"

/** @brief The default combined (metallic, roughness, AO) texture name. */
#define DEFAULT_COMBINED_TEXTURE_NAME "default_COMBINED"

/** @brief The default cube texture name. */
#define DEFAULT_CUBE_TEXTURE_NAME "default_cube"

/** @brief The default terrain texture name. */
#define DEFAULT_TERRAIN_TEXTURE_NAME "default_TERRAIN"

/**
 * @brief Initializes the texture system.
 * Should be called twice; once to get the memory requirement (passing state=0), and a second
 * time passing an allocated block of memory to actually initialize the system.
 *
 * @param memory_requirement A pointer to hold the memory requirement as it is calculated.
 * @param state A block of memory to hold the state or, if gathering the memory requirement, 0.
 * @param config The configuration (texture_system_config) for this system.
 * @return True on success; otherwise false.
 */
b8 texture_system_initialize(u64* memory_requirement, void* state, void* config);
void texture_system_shutdown(void* state);

KAPI texture* texture_system_acquire(const char* name, b8 auto_release);

/**
 * @brief Attempts to acquire a cubemap texture with the given name. If it has not yet been loaded,
 * this triggers it to load. If the texture is not found, a pointer to the default texture
 * is returned. If the texture _is_ found and loaded, its reference counter is incremented.
 * Requires textures with name as the base, one for each side of a cube, in the following order:
 * - name_f Front
 * - name_b Back
 * - name_u Up
 * - name_d Down
 * - name_r Right
 * - name_l Left
 *
 * For example, "skybox_f.png", "skybox_b.png", etc. where name is "skybox".
 *
 * @param name The name of the texture to find. Used as a base string for actual texture names.
 * @param auto_release Indicates if the texture should auto-release when its reference count is 0.
 * Only takes effect the first time the texture is acquired.
 * @return A pointer to the loaded texture. Can be a pointer to the default texture if not found.
 */
KAPI texture* texture_system_acquire_cube(const char* name, b8 auto_release);

/**
 * @brief Attempts to acquire a writeable texture with the given name. This does not point to
 * nor attempt to load a texture file. Does also increment the reference counter.
 * NOTE: Writeable textures are not auto-released.
 *
 * @param name The name of the texture to acquire.
 * @param width The texture width in pixels.
 * @param height The texture height in pixels.
 * @param channel_count The number of channels in the texture (typically 4 for RGBA)
 * @param has_transparency Indicates if the texture will have transparency.
 * @return A pointer to the generated texture.
 */
KAPI texture* texture_system_acquire_writeable(const char* name, u32 width, u32 height, u8 channel_count, b8 has_transparency);

/**
 * @brief Attempts to acquire a writeable array  texture with the given name. This does not point to
 * nor attempt to load a texture file. Does also increment the reference counter.
 * NOTE: Writeable textures are not auto-released.
 *
 * @param name The name of the texture to acquire.
 * @param width The texture width in pixels.
 * @param height The texture height in pixels.
 * @param channel_count The number of channels in the texture (typically 4 for RGBA)
 * @param has_transparency Indicates if the texture will have transparency.
 * @param type The texture type.
 * @param array_size The number of "layers" in the texture.
 * @return A pointer to the generated texture.
 */
KAPI texture* texture_system_acquire_writeable_arrayed(const char* name, u32 width, u32 height, u8 channel_count, b8 has_transparency, texture_type type, u16 array_size);

/**
 * @brief Attempts to acquire an array texture with the given name. This uses the provided array
 * of texture names to load data from each in its own layer. All textures must be be of the same size.
 * Size is determined by the first file in the list.
 *
 * @param name The name of the texture to acquire. Not tied to a filename, only used for lookups.
 * @param layer_count The number of layers in the array texture (Must be at least 1)
 * @param layer_texture_names The names of the textures to load, one per layer. All textures must be the same dimension. Size of array must match layer_count.
 * @param auto_release Indicates if the texture will have its resources automatically released when the last reference is released.
 * @return A pointer to the generated texture.
 */
texture* texture_system_acquire_textures_as_arrayed(const char* name, u32 layer_count, const char** layer_texture_names, b8 auto_release);

/**
 * @brief Releases a texture with the given name. Ignores non-existant textures.
 * Decreases the reference counter by 1. If the reference counter reaches 0 and
 * auto_release was set to true, the texture is unloaded, releasing internal resources.
 *
 * @param name The name of the texture to unload.
 */
KAPI void texture_system_release(const char* name);

/**
 * @brief Wraps the provided internal data in a texture structure using the parameters
 * provided. This is best used for when the renderer system creates internal resources
 * and they should be passed off to the texture system. Can be looked up by name via
 * the acquire methods.
 * NOTE: Wrapped textures are not auto-released.
 *
 * @param name The name of the texture.
 * @param width The texture width in pixels.
 * @param height The texture height in pixels.
 * @param channel_count The number of channels in the texture (typically 4 for RGBA)
 * @param has_transparency Indicates if the texture will have transparency.
 * @param is_writeable Indicates if the texture is writeable.
 * @param renderer_texture_handle A handle to the internal data to be set on the texture.
 * @param register_texture Indicates if the texture should be registered with the system.
 * @param out_texture An optional pointer to hold the wrapped texture. If null, a new pointer is allocated and returned instead.
 */
KAPI void texture_system_wrap_internal(const char* name, u32 width, u32 height, u8 channel_count, b8 has_transparency, b8 is_writeable, b8 register_texture, k_handle renderer_texture_handle, texture* out_texture);

/**
 * @brief Resizes the given texture. May only be done on writeable textures.
 * Potentially regenerates internal data, if configured to do so.
 *
 * @param t A pointer to the texture to be resized.
 * @param width The new width in pixels.
 * @param height The new height in pixels.
 * @param regenerate_internal_data Indicates if the internal data should be regenerated.
 * @return True on success; otherwise false.
 */
KAPI b8 texture_system_resize(texture* t, u32 width, u32 height, b8 regenerate_internal_data);

/**
 * @brief Writes the given data to the provided texture. May only be used on
 * writeable textures.
 *
 * @param t A pointer to the texture to be written to.
 * @param offset The offset in bytes from the beginning of the data to be written.
 * @param size The number of bytes to be written.
 * @param data A pointer to the data to be written.
 * @return True on success; otherwise false.
 */
KAPI b8 texture_system_write_data(texture* t, u32 offset, u32 size, void* data);

/**
 * @brief Indicates if the passed-in texture is a default texture.
 * Will return false if texture system is not yet initialized.
 *
 * @param t A pointer to the texture to be checked.
 * @returns True if t is a default texture; otherwise false.
 */
KAPI b8 texture_system_is_default_texture(texture* t);

/**
 * @brief Gets a pointer to the default texture. No reference counting is
 * done for default textures.
 */
KAPI texture* texture_system_get_default_texture(void);

/**
 * @brief Gets a pointer to the default diffuse texture. No reference counting is
 * done for default textures.
 */
KAPI texture* texture_system_get_default_diffuse_texture(void);

/**
 * @brief Gets a pointer to the default specular texture. No reference counting is
 * done for default textures.
 */
KAPI texture* texture_system_get_default_specular_texture(void);

/**
 * @brief Gets a pointer to the default normal texture. No reference counting is
 * done for default textures.
 */
KAPI texture* texture_system_get_default_normal_texture(void);

/**
 * @brief Gets a pointer to the default combined (metallic, roughness, AO) texture. No reference counting is
 * done for default textures.
 */
KAPI texture* texture_system_get_default_combined_texture(void);
/**
 * @brief Gets a pointer to the default cube texture. No reference counting is
 * done for default textures.
 */
KAPI texture* texture_system_get_default_cube_texture(void);

/**
 * @brief Gets a pointer to the default terrain texture. No reference counting is
 * done for default textures.
 */
KAPI texture* texture_system_get_default_terrain_texture(void);

/**
 * @brief Gets a pointer to either the internal data of the supplied texture if loaded,
 * or one to the internal of a default texture of the appropriate type. If a default texture
 * is used, out_generation will be set to INVALID_ID. If an invalid texture is passed, 0/null
 * will be returned.
 *
 * @param t A pointer to the texture whose internal data will be fetched.
 * @param out_generation A pointer to hold the generation of the texture.
 * @returns A pointer to texture internal data if successful, otherwise 0/null.
 */
KAPI struct texture_internal_data* texture_system_get_internal_or_default(texture* t,u8* out_generation);