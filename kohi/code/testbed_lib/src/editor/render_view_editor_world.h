#pragma once

#include "defines.h"
#include "renderer/renderer_types.inl"

struct linear_allocator;
struct frame_data;
struct editor_gizmo;

typedef struct editor_world_packet_data {
    struct editor_gizmo* gizmo;
} editor_world_packet_data;

b8 render_view_editor_world_on_registered(struct render_view* self);
void render_view_editor_world_on_destroy(struct render_view* self);
void render_view_editor_world_on_resize(struct render_view* self, u32 width, u32 height);
b8 render_view_editor_world_on_packet_build(const struct render_view* self, struct linear_allocator* frame_allocator, void* data, struct render_view_packet* out_packet);
void render_view_editor_world_on_packet_destroy(const struct render_view* self, struct render_view_packet* packet);
b8 render_view_editor_world_on_render(const struct render_view* self, const struct render_view_packet* packet, u64 frame_number, u64 render_target_index, const struct frame_data* p_frame_data);
