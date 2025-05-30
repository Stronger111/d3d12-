#pragma once

#include "vulkan_types.h"

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

