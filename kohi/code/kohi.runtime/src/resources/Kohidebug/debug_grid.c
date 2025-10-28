#include "debug_grid.h"

#include "identifiers/identifier.h"
#include "strings/kstring.h"
#include "defines.h"
#include "math/kmath.h"
#include "renderer/renderer_frontend.h"

b8 debug_grid_create(const debug_grid_config *config, debug_grid *out_grid) {
    if (!config || !out_grid) {
        return false;
    }

    kzero_memory(out_grid, sizeof(debug_grid));
    
    out_grid->name=config->name;
    out_grid->segment_count_dim_0 = config->segment_count_dim_0;
    out_grid->segment_count_dim_1 = config->segment_count_dim_1;
    out_grid->segment_size = config->segment_size;
    out_grid->orientation = config->orientation;
    out_grid->use_third_axis = config->use_third_axis;

    // f32 max_0 = out_grid->tile_count_dim_0 * out_grid->tile_scale;
    // f32 min_0 = -max_0;
    // f32 max_1 = out_grid->tile_count_dim_1 * out_grid->tile_scale;
    // f32 min_1 = -max_1;
    // out_grid->extents.min = vec3_zero();
    // out_grid->extents.max = vec3_zero();
    // switch (out_grid->orientation) {
    //     default:
    //     case DEBUG_GRID_ORIENTATION_XZ:
    //         out_grid->extents.min.x = min_0;
    //         out_grid->extents.max.x = max_0;
    //         out_grid->extents.min.z = min_1;
    //         out_grid->extents.max.z = max_1;
    //         break;
    //     case DEBUG_GRID_ORIENTATION_XY:
    //         out_grid->extents.min.x = min_0;
    //         out_grid->extents.max.x = max_0;
    //         out_grid->extents.min.y = min_1;
    //         out_grid->extents.max.y = max_1;
    //         break;
    //     case DEBUG_GRID_ORIENTATION_YZ:
    //         out_grid->extents.min.y = min_0;
    //         out_grid->extents.max.y = max_0;
    //         out_grid->extents.min.z = min_1;
    //         out_grid->extents.max.z = max_1;
    //         break;
    // }

    //FIXME: do we need this?
    out_grid->origin = vec3_zero();
    out_grid->id = identifier_create();

    // // 2 verts per line, 1 line per tile in each direction, plus one in the middle for each direction. Adding 2 more for third axis.
    // out_grid->vertex_count = ((out_grid->tile_count_dim_0 * 2 + 1) * 2) + ((out_grid->tile_count_dim_1 * 2 + 1) * 2) + 2;

    return true;
}

void debug_grid_destroy(debug_grid *grid) {
    // TODO: zero out, etc.
    grid->id.uniqueid = INVALID_ID_U64;
}

b8 debug_grid_initialize(debug_grid *grid) {
    if (!grid) {
        return false;
    }

   grid->geometry=geometry_generate_grid(grid->orientation, grid->segment_count_dim_0, grid->segment_count_dim_1, grid->segment_size, grid->use_third_axis, grid->name);

    return true;
}

b8 debug_grid_load(debug_grid *grid) {
    // Send the geometry off to the renderer to be uploaded to the GPU.
    if (!renderer_geometry_upload(&grid->geometry)) {
        return false;
    }
    return true;
}

b8 debug_grid_unload(debug_grid *grid) {
    renderer_geometry_destroy(&grid->geometry);

    grid->id.uniqueid = INVALID_ID_U64;

    return true;
}

b8 debug_grid_update(debug_grid *grid) {
    return true;
}
