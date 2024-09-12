#pragma once

#include "vulkan_types.inl"

/**
 * @brief Creates a new swapchain.
 * 
 * @param context A pointer to the Vulkan context.
 * @param width The initial width of the surface area.
 * @param height The initial height of the surface area.
 * @param vsync Indicates if the swapchain should use vsync.
 * @param out_swapchain A pointer to the newly-created swapchain.
 */
void vulkan_swapchain_create(vulkan_context* context, u32 width, u32 height, renderer_config_flags flags, vulkan_swapchain* out_swapchain);

void vulkan_swapchain_recreate(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* swapchain);

void vulkan_swapchain_destroy(vulkan_context* context, vulkan_swapchain* swapchain);

b8 vulkan_swapchain_acquire_next_image_index(vulkan_context* context, vulkan_swapchain* swapchain,
 u64 timeout_ns,VkSemaphore image_available_semaphore, VkFence fence, u32* out_image_index);

/**
 * @brief Presents the swapchain's current image to the surface.
 * 
 * @param context A pointer to the Vulkan context.
 * @param swapchain A pointer to the swapchain to present.
 * @param present_queue The presentation queue used for presentation.
 * @param render_complete_semaphore The semaphore that will be signaled when the presentation is complete.
 * @param present_image_index The image index to present.
 */
 void vulkan_swapchain_present(vulkan_context* context,
  vulkan_swapchain* swapchain,VkQueue present_queue,VkSemaphore render_complete_semaphore,
   u32 present_image_index);