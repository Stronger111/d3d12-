
#ifndef _EDITOR_PASS_H_
#define _EDITOR_PASS_H_

#include "defines.h"

struct rendergraph_node;
struct frame_data;

struct geometry_render_data;

typedef struct editor_pass_extended_data {
    u32 debug_geometry_count;
    struct geometry_render_data* debug_geometries;
} editor_pass_extended_data;

b8 editor_pass_create(struct rendergraph_node* self,void* config);
b8 editor_pass_initialize(struct rendergraph_node* self);
b8 editor_pass_execute(struct rendergraph_node* self, struct frame_data* p_frame_data);
void editor_pass_destroy(struct rendergraph_node* self);

#endif
