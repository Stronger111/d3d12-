#pragma once

#include "defines.h"

typedef enum  memory_tag
{
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT, //字典
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFROM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,
    MEMORY_TAG_RESOURCE,

    MEMORY_TAG_MAX_TAGS
}memory_tag;

/** @brief The configuration for the memory system. */
typedef struct memory_system_configuration
{
    /** @brief The total memory size in byes used by the internal allocator for this system. */
    u64 total_alloc_size;
}memory_system_configuration;

/**
 * @brief Initializes the memory system.
 * @param config The configuration for this system.
 */
KAPI b8 memory_system_initialize(memory_system_configuration config);
/**
 * @brief Shuts down the memory system.
 */
KAPI void memory_system_shutdown();

KAPI void* kallocate(u64 size,memory_tag tag);

KAPI void kfree(void* block,u64 size,memory_tag tag);
//内存大小清零
KAPI void* kzero_memory(void* block,u64 size);

KAPI void* kcopy_memory(void* dest,const void* source,u64 size);

KAPI void* kset_memory(void* dest,i32 value,u64 size);
//usage 用法用量
KAPI char* get_memory_usage_str();

KAPI u64 get_memory_alloc_count();