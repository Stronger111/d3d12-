#pragma once

#include "math/math_types.h"

// Pre-defined resource types
typedef enum resource_type {
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    /** @brief Shader resource type (or more accurately shader config). */
    RESOURCE_TYPE_SHADER,
    /** @brief Mesh resource type (collection of geometry configs). */
    RESOURCE_TYPE_MESH,
    /** @brief Custom resource type. Used by loaders outside the core engine. */
    RESOURCE_TYPE_CUSTOM
} resource_type;

typedef struct resource {
    u32 loader_id;
    const char* name;
    char* full_path;
    u64 data_size;
    void* data;
} resource;

typedef struct image_resource_data {
    u8 channel_count;
    u32 width;
    u32 height;
    u8* pixels;
} image_resource_data;

#define TEXTURE_NAME_MAX_LENGTH 512

typedef struct texture {
    u32 id;
    u32 width;
    u32 height;
    u8 channel_count;
    b8 has_transparency;
    /** @brief Indicates if the texture can be written (rendered) to. */
    b8 is_writeable;
    u32 generation;
    char name[TEXTURE_NAME_MAX_LENGTH];
    void* internal_data;
} texture;

typedef enum texture_use {
    TEXTURE_USE_UNKNOWN = 0x00,
    /** @brief The texture is used as a diffuse map. */
    TEXTURE_USE_MAP_DIFFUSE = 0x01,
    /** @brief The texture is used as a specular map. */
    TEXTURE_USE_MAP_SPECULAR = 0x02,
    /** @brief The texture is used as a normal map. */
    TEXTURE_USE_MAP_NORMAL = 0x03
} texture_use;

/** @brief Represents supported the texture filtering modes. */
typedef enum texture_filter {
    /** @brief Nearest-neighbor filtering. */
    TEXTURE_FILTER_MODE_NEAREST = 0x0,
    /** @brief Linear (i.e. bilinear) filtering.*/
    TEXTURE_FILTER_MODE_LINEAR = 0x1,
} texture_filter;

typedef enum texture_repeat {
    TEXTURE_REPEAT_REPEAT = 0x1,
    TEXTURE_REPEAT_MIRRORED_REPEAT = 0x2,
    TEXTURE_REPEAT_CLAMP_TO_EDGE = 0x3,
    TEXTURE_REPEAT_CLAMP_TO_BORDER = 0x4,
} texture_repeat;

typedef struct texture_map {
    texture* texture;
    texture_use use;
    /** @brief Texture filtering mode for minification. */
    texture_filter filter_minify;
    /** @brief Texture filtering mode for magnification. */
    texture_filter filter_magnify;
    /** @brief The repeat mode on the U axis (or X, or S) */
    texture_repeat repeat_u;
    /** @brief The repeat mode on the V axis (or Y, or T) */
    texture_repeat repeat_v;
    /** @brief The repeat mode on the W axis (or Z, or R) */
    texture_repeat repeat_w;
    /** @brief A pointer to internal, render API-specific data. Typically the internal sampler. */
    void* internal_data;
} texture_map;

#define MATERIAL_NAME_MAX_LENGTH 256

/**
 * @brief Material configuration typically loaded from
 * a file or created in code to load a material from.
 */
typedef enum material_type {
    MATERIAL_TYPE_WORLD,
    MATERIAL_TYPE_UI
} material_type;

typedef struct material_config {
    char name[MATERIAL_NAME_MAX_LENGTH];
    char* shader_name;
    b8 auto_release;
    vec4 diffuse_colour;
    /** @brief The shininess of the material. */
    f32 shininess;
    char diffuse_map_name[TEXTURE_NAME_MAX_LENGTH];
    /** @brief The specular map name. */
    char specular_map_name[TEXTURE_NAME_MAX_LENGTH];
    /** @brief The normal map name. */
    char normal_map_name[TEXTURE_NAME_MAX_LENGTH];
} material_config;

typedef struct material {
    u32 id;
    u32 generation;
    u32 internal_id;
    char name[MATERIAL_NAME_MAX_LENGTH];
    vec4 diffuse_colour;
    texture_map diffuse_map;
    /** @brief The specular texture map. */
    texture_map specular_map;
    /** @brief The normal texture map. */
    texture_map normal_map;
    /** @brief The material shininess, determines how concentrated the specular lighting is. */
    f32 shininess;
    u32 shader_id;
    /** @brief Synced to the renderer's current frame number when the material has been applied that frame. */
    u32 render_frame_number;
} material;

#define GEOMETRY_NAME_MAX_LENGTH 256

/**
 * @brief Represents actual geometry in the world.
 * Typically (but not always, depending on use) paired with a material.
 */
typedef struct geometry {
    u32 id;
    u32 internal_id;
    u32 generation;
    char name[GEOMETRY_NAME_MAX_LENGTH];
    material* material;
} geometry;

typedef struct mesh {
    u16 geometry_count;
    geometry** geometries;
    transform transform;
} mesh;

/** @brief Shader stages available in the system. */
typedef enum shader_stage {
    SHADER_STAGE_VERTEX = 0x00000001,
    SHADER_STAGE_GEOMETRY = 0x00000002,
    SHADER_STAGE_FRAGMENT = 0x00000004,
    SHADER_STAGE_COMPUTE = 0x0000008
} shader_stage;

/** @brief Available attribute types. */
typedef enum shader_attribute_type {
    SHADER_ATTRIB_TYPE_FLOAT32 = 0U,
    SHADER_ATTRIB_TYPE_FLOAT32_2 = 1U,
    SHADER_ATTRIB_TYPE_FLOAT32_3 = 2U,
    SHADER_ATTRIB_TYPE_FLOAT32_4 = 3U,
    SHADER_ATTRIB_TYPE_MATRIX_4 = 4U,
    SHADER_ATTRIB_TYPE_INT8 = 5U,
    SHADER_ATTRIB_TYPE_UINT8 = 6U,
    SHADER_ATTRIB_TYPE_INT16 = 7U,
    SHADER_ATTRIB_TYPE_UINT16 = 8U,
    SHADER_ATTRIB_TYPE_INT32 = 9U,
    SHADER_ATTRIB_TYPE_UINT32 = 10U,
} shader_attribute_type;

/** @brief Available uniform types. */
typedef enum shader_uniform_type {
    SHADER_UNIFORM_TYPE_FLOAT32 = 0U,
    SHADER_UNIFORM_TYPE_FLOAT32_2 = 1U,
    SHADER_UNIFORM_TYPE_FLOAT32_3 = 2U,
    SHADER_UNIFORM_TYPE_FLOAT32_4 = 3U,
    SHADER_UNIFORM_TYPE_INT8 = 4U,
    SHADER_UNIFORM_TYPE_UINT8 = 5U,
    SHADER_UNIFORM_TYPE_INT16 = 6U,
    SHADER_UNIFORM_TYPE_UINT16 = 7U,
    SHADER_UNIFORM_TYPE_INT32 = 8U,
    SHADER_UNIFORM_TYPE_UINT32 = 9U,
    SHADER_UNIFORM_TYPE_MATRIX_4 = 10U,
    SHADER_UNIFORM_TYPE_SAMPLER = 11U,
    SHADER_UNIFORM_TYPE_CUSTOM = 255U
} shader_uniform_type;
/**
 * @brief Defines shader scope, which indicates how
 * often it gets updated.
 */
typedef enum shader_scope {
    /** @brief Global shader scope, generally updated once per frame. */
    SHADER_SCOPE_GLOBAL = 0,
    /** @brief Instance shader scope, generally updated "per-instance" of the shader. */
    SHADER_SCOPE_INSTANCE = 1,
    /** @brief Local shader scope, generally updated per-object */
    SHADER_SCOPE_LOCAL = 2
} shader_scope;

/** @brief Configuration for an attribute. */
typedef struct shader_attribute_config {
    /** @brief The length of the name. */
    u8 name_length;
    /** @brief The name of the attribute. */
    char* name;
    /** @brief The size of the attribute. */
    u8 size;
    /** @brief The type of the attribute. */
    shader_attribute_type type;
} shader_attribute_config;

/** @brief Configuration for a uniform. */
typedef struct shader_uniform_config {
    /** @brief The length of the name. */
    u8 name_length;
    /** @brief The name of the uniform. */
    char* name;
    /** @brief The size of the uniform. */
    u8 size;
    /** @brief The location of the uniform. */
    u32 location;
    /** @brief The type of the uniform. */
    shader_uniform_type type;
    /** @brief The scope of the uniform. */
    shader_scope scope;
} shader_uniform_config;

/**
 * @brief Configuration for a shader. Typically created and
 * destroyed by the shader resource loader, and set to the
 * properties found in a .shadercfg resource file.
 */
typedef struct shader_config {
    /** @brief The name of the shader to be created. */
    char* name;

    /** @brief Indicates if the shader uses instance-level uniforms. */
    b8 use_instances;
    /** @brief Indicates if the shader uses local-level uniforms. */
    b8 use_local;
    /** @brief The count of attributes. */
    u8 attribute_count;
    /** @brief The collection of attributes. Darray. */
    shader_attribute_config* attributes;
    /** @brief The count of uniforms. */
    u8 uniform_count;
    /** @brief The collection of uniforms. Darray. */
    shader_uniform_config* uniforms;
    /** @brief The name of the renderpass used by this shader. */
    char* renderpass_name;
    /** @brief The number of stages present in the shader. */
    u8 stage_count;
    /** @brief The collection of stages. Darray. */
    shader_stage* stages;
    /** @brief The collection of stage names. Must align with stages array. Darray. */
    char** stage_names;
    /** @brief The collection of stage file names to be loaded (one per stage). Must align with stages array. Darray. */
    char** stage_filenames;
} shader_config;