#pragma once

#include "renderer_types.inl"

b8 renderer_system_initialize(u64* memory_requirement,void* state, char* application_name);
void renderer_system_shutdown(void* state);

b8 renderer_begin_frame(f32 delta_time);
b8 renderer_end_frame(f32 delta_time);

void renderer_on_resized(u16 width,u16 height);

b8 renderer_draw_frame(render_packet* packet);