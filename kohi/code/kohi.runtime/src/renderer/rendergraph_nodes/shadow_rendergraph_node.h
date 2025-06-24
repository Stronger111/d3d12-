#ifndef _SHADOW_RENDERGRAPH_PASS_H_
#define _SHADOW_RENDERGRAPH_PASS_H_

#include "defines.h"
#include "math/math_types.h"
#include "renderer/renderer_types.h"

struct rendergraph_node;
struct rendergraph_source;
struct frame_data;

#define MAX_CASCADE_COUNT 4

typedef struct shadow_map_cascade_data {
    mat4 projection;
    mat4 view;
    f32 split_depth;
    i32 cascade_index;
} shadow_map_cascade_data;

typedef struct shadow_map_pass_extended_data {
    struct directional_light* light;
    // Per -cascade data.
    shadow_map_cascade_data cascades[MAX_CASCADE_COUNT];

    u32 terrain_geometry_count;
    struct geometry_render_data* terrain_geometries;
    u32 geometry_count;
    struct geometry_render_data* geometries;
} shadow_map_pass_extended_data;

typedef struct shadow_rendergraph_node_config {
    u16 resolution;
} shadow_rendergraph_node_config;

KAPI b8 shadow_rendergraph_node_create(struct rendergraph_node* self, void* config);
KAPI b8 shadow_rendergraph_node_initialize(struct rendergraph_node* self);
KAPI b8 shadow_rendergraph_node_load_resources(struct rendergraph_node* self);
KAPI b8 shadow_rendergraph_node_execute(struct rendergraph_node* self, struct frame_data* p_frame_data);
KAPI void shadow_rendergraph_node_destroy(struct rendergraph_node* self);

// KAPI b8 shadow_map_pass_source_populate(struct rendergraph_node* self, struct rendergraph_source* source);
// KAPI b8 shadow_map_pass_attachment_populate(struct rendergraph_node* self, render_target_attachment* attachment);
#endif