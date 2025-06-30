#ifndef _FORWARD_RENDERGRAPH_NODE_H_
#define _FORWARD_RENDERGRAPH_NODE_H_

#include "defines.h"
#include "math/math_types.h"

struct rendergraph_node;
struct frame_data;
struct texture;

struct directional_light;
struct geometry_render_data;
struct viewport;

// typedef struct scene_pass_extended_data {
//     u32 render_mode;
//     vec4 cascade_splits;

//     u32 geometry_count;
//     struct geometry_render_data* geometries;

//     u32 terrain_geometry_count;
//     struct geometry_render_data* terrain_geometries;

//     u32 debug_geometry_count;
//     struct geometry_render_data* debug_geometries;

//     struct texture* irradiance_cube_texture;

//     mat4 directional_light_views[MAX_CASCADE_COUNT];
//     mat4 directional_light_projections[MAX_CASCADE_COUNT];
// } scene_pass_extended_data;

KAPI b8 forward_rendergraph_node_create(struct rendergraph_node* self, void* config);
KAPI b8 forward_rendergraph_node_initialize(struct rendergraph_node* self);
KAPI b8 forward_rendergraph_node_load_resources(struct rendergraph_node* self);
KAPI b8 forward_rendergraph_node_execute(struct rendergraph_node* self, struct frame_data* p_frame_data);
KAPI void forward_rendergraph_node_destroy(struct rendergraph_node* self);

KAPI b8 forward_rendergraph_node_render_mode_set(struct rendergraph_node* self, u32 render_mode);
KAPI b8 forward_rendergraph_node_directional_light_set(struct rendergraph_node* self, struct directional_light* light);
KAPI b8 forward_rendergraph_node_cascade_data_set(struct rendergraph_node* self, f32 split, mat4 dir_light_view, mat4 dir_light_projection, u8 cascade_index);
KAPI b8 forward_rendergraph_node_static_geometries_set(struct rendergraph_node* self, struct frame_data* p_frame_data, u32 geometry_count, const struct geometry_render_data* geometries);
KAPI b8 forward_rendergraph_node_dynamic_geometries_set(struct rendergraph_node* self, struct frame_data* p_frame_data, u32 geometry_count, const struct geometry_render_data* geometries);
KAPI b8 forward_rendergraph_node_terrain_geometries_set(struct rendergraph_node* self, struct frame_data* p_frame_data, u32 geometry_count, const struct geometry_render_data* geometries);
KAPI b8 forward_rendergraph_node_debug_geometries_set(struct rendergraph_node* self, struct frame_data* p_frame_data, u32 geometry_count, const struct geometry_render_data* geometries);
KAPI b8 forward_rendergraph_node_irradiance_texture_set(struct rendergraph_node* self, struct frame_data* p_frame_data, struct texture* irradiance_cube_texture);

KAPI b8  forward_rendergraph_node_viewport_set(struct rendergraph_node* self, struct viewport* v);
#endif
