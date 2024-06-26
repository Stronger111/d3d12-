#pragma once 

#include "renderer/renderer_backend.h"

b8 vulkan_renderer_backend_initialize(struct renderer_backend* backend,const char* application_name);
void vulkan_renderer_backend_shutdown(struct renderer_backend* backend);

void vulkan_renderer_backend_on_resized(struct renderer_backend* backend,u16 width,u16 height);

b8 vulkan_renderer_backend_begin_frame(struct renderer_backend* backend,f32 delta_time);

b8 vulkan_renderer_backend_end_frame(struct renderer_backend* backend,f32 delta_time);