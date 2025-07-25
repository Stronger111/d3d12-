/**
 * @file shader_system.h
 * @author Travis Vroman (travis@kohiengine.com)
 * @brief A system to manage shaders. Respondible for working with the
 * renderer to create, destroy, bind/unbind and set shader properties
 * such as uniforms.
 * @version 1.0
 * @date 2022-03-09
 *
 * @copyright Kohi Game Engine is Copyright (c) Travis Vroman 2021-2022
 *
 */
#pragma once

#include "containers/hashtable.h"
#include "defines.h"
#include "renderer/renderer_types.h"
#include "resources/resource_types.h"

 /** @brief Configuration for the shader system. */
typedef struct shader_system_config {
    /** @brief The maximum number of shaders held in the system. NOTE: Should be at least 512. */
    u16 max_shader_count;
    /** @brief The maximum number of uniforms allowed in a single shader. */
    u8 max_uniform_count;
    /** @brief The maximum number of global-scope textures allowed in a single shader. */
    u8 max_global_textures;
    /** @brief The maximum number of instance-scope textures allowed in a single shader. */
    u8 max_instance_textures;
} shader_system_config;

/**
 * @brief Represents the current state of a given shader.
 */
typedef enum shader_state {
    /** @brief The shader has not yet gone through the creation process, and is unusable.*/
    SHADER_STATE_NOT_CREATED,
    /** @brief The shader has gone through the creation process, but not initialization. It is unusable.*/
    SHADER_STATE_UNINITIALIZED,
    /** @brief The shader is created and initialized, and is ready for use.*/
    SHADER_STATE_INITIALIZED,
} shader_state;

/**
 * @brief Represents a single entry in the internal uniform array.
 */
typedef struct shader_uniform {
    /** @brief The offset in bytes from the beginning of the uniform set (global/instance/local) */
    u64 offset;
    /**
     * @brief The location to be used as a lookup. Typically the same as the index except for samplers,
     * which is used to lookup texture index within the internal array at the given scope (global/instance).
     */
    u16 location;
    /** @brief Index into the internal uniform array. */
    u16 index;
    /** @brief The size of the uniform, or 0 for samplers. */
    u16 size;
    /** @brief The index of the descriptor set the uniform belongs to (0=global, 1=instance, INVALID_ID=local). */
    u8 set_index;
    /** @brief The scope of the uniform. */
    shader_scope scope;
    /** @brief The type of uniform. */
    shader_uniform_type type;
    /** @brief The length of the array if it is one; otherwise 0 */
    u32 array_length;
} shader_uniform;

/**
 * @brief Represents a single shader vertex attribute.
 */
typedef struct shader_attribute {
    /** @brief The attribute name. */
    char* name;
    /** @brief The attribute type. */
    shader_attribute_type type;
    /** @brief The attribute size in bytes. */
    u32 size;
} shader_attribute;

typedef enum shader_flags {
    SHADER_FLAG_NONE = 0x00,
    //Reads from depth buffer.
    SHADER_FLAG_DEPTH_TEST = 0x01,
    //Writes to depth buffer.
    SHADER_FLAG_DEPTH_WRITE = 0x02,
    // 线框
    SHADER_FLAG_WIREFRAME = 0x04,
    //Reads from depth buffer.
    SHADER_FLAG_STENCIL_TEST = 0x08,
    //Writes to depth buffer.
    SHADER_FLAG_STENCIL_WRITE = 0x10,
    //Reads from colour buffer.
    SHADER_FLAG_COLOUR_READ = 0x20,
    //Writes to colour buffer.
    SHADER_FLAG_COLOUR_WRITE = 0x40,
} shader_flags;

typedef u32 shader_flag_bits;

/**
 * @brief Represents a shader on the frontend.
 */
typedef struct shader {
    /** @brief The shader identifier */
    u32 id;

    char* name;

    shader_flag_bits flags;
    /** @brief The types of topologies used by the shader and its pipeline. See primitive_topology_type. */
    u32 topology_types;
    /**
     * @brief The amount of bytes that are required for UBO alignment.
     *
     * This is used along with the UBO size to determine the ultimate
     * stride, which is how much the UBOs are spaced out in the buffer.
     * For example, a required alignment of 256 means that the stride
     * must be a multiple of 256 (true for some nVidia cards).
     */
    u64 required_ubo_alignment;
    /** @brief The actual size of the global uniform buffer object. */
    u64 global_ubo_size;
    /** @brief The stride of the global uniform buffer object. */
    u64 global_ubo_stride;
    /**
     * @brief The offset in bytes for the global UBO from the beginning
     * of the uniform buffer.
     */
    u64 global_ubo_offset;
    /** @brief The actual size of the instance uniform buffer object. */
    u64 ubo_size;
    /** @brief The stride of the instance uniform buffer object. */
    u64 ubo_stride;

    u64 local_ubo_offset;
    u64 local_ubo_size;
    u64 local_ubo_stride;
    /** @brief An array of global texture pointers. Darray */
    texture_map** global_texture_maps;  // 数组
    /** @brief The number of instance textures. */
    u8 instance_texture_count;

    /** @brief The identifier of the currently bound instance. */
    u32 bound_instance_id;

    /** @brief The block of memory used by the uniform hashtable. */
    void* hashtable_block;
    /** @brief A hashtable to store uniform index/locations by name. */
    hashtable uniform_lookup;
    /** @brief An array of uniforms in this shader. Darray. */
    shader_uniform* uniforms;

    /** @brief The number of global non-sampler uniforms. */
    u8 global_uniform_count;

    /** @brief The number of global sampler uniforms. */
    u8 global_uniform_sampler_count;

    // darray Keeps the uniform indices of global samplers for fast lookups.
    u32* global_sampler_indices;

    /** @brief The number of instance non-sampler uniforms. */
    u8 instance_uniform_count;

    /** @brief The number of instance sampler uniforms. */
    u8 instance_uniform_sampler_count;
    // darray Keeps the uniform indices of instance samplers for fast lookups.
    u32* instance_sampler_indices;

    /** @brief The number of local non-sampler uniforms. */
    u8 local_uniform_count;

    /** @brief An array of attributes. Darray. */
    shader_attribute* attributes;
    /** @brief The internal state of the shader. */
    shader_state state;
    /** @brief The size of all attributes combined, a.k.a. the size of a vertex. */
    u16 attribute_stride;

    u8 shader_stage_count;
    shader_stage_config* stage_configs;

    /** @brief Indicates if the shader is currently flagged to use wireframe. */
    b8 is_wireframe;

    /** @brief An opaque pointer to hold renderer API specific data. Renderer is responsible for creation and destruction of this.  */
    void* internal_data;

#ifdef _DEBUG
    u32* module_watch_ids;
#endif

} shader;

/**
 * @brief Initializes the shader system using the supplied configuration.
 * NOTE: Call this twice, once to obtain memory requirement (memory = 0) and a second time
 * including allocated memory.
 *
 * @param memory_requirement A pointer to hold the memory requirement of this system in bytes.
 * @param memory A memory block to be used to hold the state of this system. Pass 0 on the first call to get memory requirement.
 * @param config The configuration (shader_system_config) to be used when initializing the system.
 * @return b8 True on success; otherwise false.
 */
b8 shader_system_initialize(u64* memory_requirement, void* memory, void* config);

/**s
 * @brief Shuts down the shader system.
 *
 * @param state A pointer to the system state.
 */
void shader_system_shutdown(void* state);

/**
 * @brief Creates a new shader with the given config.
 * @param config The configuration to be used when creating the shader.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_create(const shader_config* config);

/**
 * @brief Reloads the given shader.
 *
 * @param shader_id The id of the shader to reload.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_reload(u32 shader_id);

/**
 * @brief Gets the identifier of a shader by name.
 *
 * @param shader_name The name of the shader.
 * @return The shader id, if found; otherwise INVALID_ID.
 */
KAPI u32 shader_system_get_id(const char* shader_name);

/**
 * @brief Returns a pointer to a shader with the given identifier.
 *
 * @param shader_id The shader identifier.
 * @return A pointer to a shader, if found; otherwise 0.
 */
KAPI shader* shader_system_get_by_id(u32 shader_id);

/**
 * @brief Returns a pointer to a shader with the given name.
 * Attempts to load the shader if not already loaded.
 *
 * @param shader_name The name to search for. Case sensitive.
 * @return A pointer to a shader, if found/loaded; otherwise 0.
 */
KAPI shader* shader_system_get(const char* shader_name);

/**
 * @brief Attempts to set wireframe mode on the given shader. If the renderer backend, or the shader
 * does not support this , it will fail when attempting to enable. Disabling will always succeed.
 *
 * @param shader_id The id of the shader to set wireframe mode for.
 * @param wireframe_enabled Indicates if wireframe mode should be enabled.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_set_wireframe(u32 shader_id, b8 wireframe_enabled);

/**
 * @brief Uses the shader with the given identifier.
 *
 * @param shader_id The identifier of the shader to be used.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_use_by_id(u32 shader_id);

/**
 * @brief Returns the uniform location for a uniform with the given name, if found.
 *
 * @param shader_id The id of the shader to obtain the location from.
 * @param uniform_name The name of the uniform to search for.
 * @return The uniform location, if found; otherwise INVALID_ID_U16.
 */
KAPI u16 shader_system_uniform_location(u32 shader_id, const char* uniform_name);

/**
 * @brief Sets the value of a uniform with the given name to the supplied value.
 *
 * @param shader_id The identifier of the shader to update.
 * @param uniform_name The name of the uniform to be set.
 * @param value The value to be set.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_uniform_set(u32 shader_id,const char* uniform_name, const void* value);

/**
 * @brief Sets the value of an arrayed uniform with the given name to the supplied value.
 *
 * @param shader_id The identifier of the shader to update.
 * @param uniform_name The name of the uniform to be set.
 * @param array_index The index into the uniform array, if the uniform is in fact an array. Otherwise ignored.
 * @param value The value to be set.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_uniform_set_arrayed(u32 shader_id, const char* uniform_name, u32 array_index, const void* value);

/**
 * @brief Sets the texture of a sampler with the given name to the supplied texture.
 *
 * @param shader_id The identifier of the shader to update.
 * @param uniform_name The name of the uniform to be set.
 * @param t A pointer to the texture to be set.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_sampler_set(u32 shader_id, const char* sampler_name, const texture* t);

/**
 * @brief Sets the texture of an arrayed sampler with the given name to the supplied texture.
 *
 * @param shader_id The identifier of the shader to update.
 * @param uniform_name The name of the uniform to be set.
 * @param array_index The index into the uniform array, if the uniform is in fact an array. Otherwise ignored.
 * @param t A pointer to the texture to be set.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_sampler_set_arrayed(u32 shader_id, const char* sampler_name, u32 array_index, const texture* t);

/**
 * @brief Sets a uniform value by location.
 *
 * @param shader_id The identifier of the shader to update.
 * @param index The location of the uniform.
 * @param value The value of the uniform.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_uniform_set_by_location(u32 shader_id,u16 location, const void* value);

/**
 * @brief Sets a sampler value by location.
 *
 * @param shader_id The identifier of the shader to update.
 * @param location The location of the uniform.
 * @param value A pointer to the texture to be set.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_uniform_set_by_location_arrayed(u32 shader_id,u16 location, u32 array_index, const void* value);

/**
 * @brief Sets a sampler value by location.
 *
 * @param shader_id The identifier of the shader to update.
 * @param index The location of the uniform.
 * @param array_index The index into the uniform array, if the uniform is in fact an array. Otherwise ignored.
 * @param value A pointer to the texture to be set.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_sampler_set_by_location(u32 shader_id,u16 local, const struct texture* t);

/**
 * @brief Binds the instance with the given id for use. Must be done before setting
 * instance-scoped uniforms.
 *
 * @param shader_id The identifier of the shader to update.
 * @param instance_id The identifier of the instance to bind.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_bind_instance(u32 shader_id, u32 instance_id);

/**
 * @brief Applies global-scoped uniforms.
 *
 * @param shader_id The identifier of the shader to update.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_apply_global(u32 shader_id);

/**
 * @brief Applies instance-scoped uniforms.
 *
 * @param shader_id The identifier of the shader to update.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_apply_instance(u32 shader_id);

/**
 * @brief Applies local-scoped uniforms.
 *
 * @param shader_id The identifier of the shader to update.
 * @return True on success; otherwise false.
 */
KAPI b8 shader_system_apply_local(u32 shader_id);

