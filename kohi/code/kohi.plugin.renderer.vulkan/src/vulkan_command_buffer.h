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
 * @param secondary_buffer_count The number of secondary buffers to create. 0 means create none.
 */
void vulkan_command_buffer_allocate(
        vulkan_context* context,
        VkCommandPool pool,
        b8 is_primary,
        const char* name,
        vulkan_command_buffer* out_command_buffer,
        u32 secondary_buffer_count);

void vulkan_command_buffer_free(vulkan_context* context,
        VkCommandPool pool, vulkan_command_buffer* command_buffer);

void vulkan_command_buffer_begin(vulkan_command_buffer* command_buffer, b8 is_single_use, b8 is_renderpass_continue, b8 is_simultaneous_use);

void vulkan_command_buffer_end(vulkan_command_buffer* command_buffer);

/**
 * @brief Sets the command buffer to the submitted state.
 *
 * @param command_buffer A pointer to the command buffer whose state to set.
 */

 /**
  * @brief Submits the command buffer to the given queue for execution. Also sets the command buffer to the submitted state.
  *
  * @param command_buffer A pointer to the command buffer to be submitted.
  * @param queue The queue to submit to.
  * @param signal_semaphore_count The number of semaphore(s) to be signaled when the queue is complete.
  * @param signal_semaphores The semaphore(s) to be signaled when the queue is complete.
  * @param wait_semaphore_count The number of semaphore(s) to wait on before the command buffer is executed.
  * @param wait_semaphores The semaphore(s) to be waited on before the command buffer is executed.
  * @param fence An optional handle to a fence to be signaled once all submitted command buffers have completed execution.
  * @return b8 True on success; otherwise false.
  */
b8 vulkan_command_buffer_submit(
        vulkan_command_buffer* command_buffer,
        VkQueue queue,
        u32 signal_semaphore_count,
        VkSemaphore* signal_semaphores,
        u32 wait_semaphore_count,
        VkSemaphore* wait_semaphores,
        VkFence fence
);

/**
 * @brief Executes commands in the given secondary command buffer.
 *
 * @param secondary A pointer to the secondary command buffer to execute commands within.
 */
void vulkan_command_buffer_execute_secondary(vulkan_command_buffer* secondary);

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