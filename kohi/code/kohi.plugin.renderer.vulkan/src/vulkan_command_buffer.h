#pragma once
#include "vulkan_types.h"

/**
 * @brief Allocates a new command buffer from the given pool.
 *
 * @param context A pointer to the Vulkan context.
 * @param pool The pool to allocate a command buffer from.
 * @param is_primary Indicates if the command buffer is a primary or secondary buffer.
 * @param name The name of the command buffer, for debugging purposes.
 * @param out_command_buffer A pointer to hold the newly allocated command buffer.
 */
void vulkan_command_buffer_allocate(
        vulkan_context* context,
        VkCommandPool pool,
        b8 is_primary,
        const char* name,
        vulkan_command_buffer* out_command_buffer);

void vulkan_command_buffer_free(vulkan_context* context,
        VkCommandPool pool, vulkan_command_buffer* command_buffer);

void vulkan_command_buffer_begin(vulkan_command_buffer* command_buffer, b8 is_single_use, b8 is_renderpass_continue, b8 is_simultaneous_use);

void vulkan_command_buffer_end(vulkan_command_buffer* command_buffer);

void vulkan_command_buffer_update_submitted(vulkan_command_buffer* command_buffer);

void vulkan_command_buffer_reset(vulkan_command_buffer* command_buffer);

/**
 * @brief Allocates and begins recording to out_command_buffer.
 *
 */
void vulkan_command_buffer_allocate_and_begin_single_use(vulkan_context* context, VkCommandPool pool, vulkan_command_buffer* out_command_buffer);

/*
*  Ends recording,submits to and waits for queue operation and frees the provided command buffer.
*/
void vulkan_command_buffer_end_single_use(vulkan_context* context, VkCommandPool pool, vulkan_command_buffer* command_buffer, VkQueue queue);