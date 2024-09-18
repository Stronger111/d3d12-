#pragma once

#include "vulkan_types.inl"
/**
 * @brief Creates a new Vulkan pipeline.
 *
 * @param context A pointer to the Vulkan context.
 * @param config A constant pointer to configuration to be used in creating the pipeline.
 * @param out_pipeline A pointer to hold the newly-created pipeline.
 * @return True on success; otherwise false.
 */
b8 vulkan_graphics_pipeline_create(vulkan_context* context,const vulkan_pipeline_config* config, vulkan_pipeline* out_pipeline);

void vulkan_pipeline_destroy( vulkan_context* context,vulkan_pipeline* pipeline);

void vulkan_pipeline_bind(vulkan_command_buffer* command_buffer,VkPipelineBindPoint bind_point, vulkan_pipeline* pipeline);