#include "standard_ui_system.h"

#include <containers/darray.h>
#include <core/event.h>
#include <core/identifier.h>
#include <core/input.h>
#include <core/kmemory.h>
#include <core/kstring.h>
#include <core/logger.h>
#include <core/systems_manager.h>
#include <defines.h>
#include <math/geometry_utils.h>
#include <math/kmath.h>
#include <math/transform.h>
#include <renderer/renderer_frontend.h>
#include <resources/resource_types.h>
#include <systems/geometry_system.h>
#include <systems/shader_system.h>
#include <systems/texture_system.h>

#include "math/math_types.h"
#include "renderer/renderer_types.h"
#include "systems/font_system.h"

// static b8 standard_ui_system_mouse_down(u16 code, void* sender, void* listener_inst, event_context context) {
//     standard_ui_state* typed_state = (standard_ui_state*)listener_inst;

//     sui_mouse_event evt;
//     evt.mouse_button = (buttons)context.data.i16[0];
//     evt.x = context.data.i16[1];
//     evt.y = context.data.i16[2];
//     for (u32 i = 0; i < typed_state->active_control_count; i++) {
//         sui_control* control = typed_state->active_controls[i];
//         if (control->internal_mouse_down || control->on_mouse_down) {
//             mat4 model = transform_world_get(&control->xform);
//             // 世界到模型矩阵
//             mat4 inv = mat4_inverse(model);
//             vec3 transformed_evt = vec3_transform((vec3){evt.x, evt.y, 0.0f}, 1.0f, inv);
//             if (rect_2d_contains_point(control->bounds, (vec2){transformed_evt.x, transformed_evt.y})) {
//                 control->is_pressed = true;
//                 if (control->internal_mouse_down) {
//                     control->internal_mouse_down(control, evt);
//                 }
//                 if (control->on_mouse_down) {
//                     control->on_mouse_down(control, evt);
//                 }
//             }
//         }
//     }

//     return false;
// }

KAPI b8 standard_ui_system_initialize(u64* memory_requirement, void* state, void* config) {
    if (!memory_requirement) {
        KERROR("standard_ui_system_initialize requires a vaild pointer to memory_requirement.");
        return false;
    }
    standard_ui_system_config* typed_config = (standard_ui_system_config*)config;
    if (typed_config->max_control_count == 0) {
        KFATAL("standard_ui_system_initialize - config.max_control_count must be > 0.");
        return false;
    }

    // Memory layout : struct + active control array + inactive_control_array
    u64 struct_requirement = sizeof(standard_ui_state);
    u64 active_array_requirement = sizeof(sui_control) * typed_config->max_control_count;
    u64 inactive_array_requirement = sizeof(sui_control) * typed_config->max_control_count;
    *memory_requirement = struct_requirement + active_array_requirement + inactive_array_requirement;

    if (!state) {
        return true;
    }

    standard_ui_state* typed_state = (standard_ui_state*)state;
    typed_state->config = *typed_config;
    // 数组初使位置坐标
    typed_state->active_controls = (void*)((u8*)state + struct_requirement);
    kzero_memory(typed_state->active_controls, sizeof(sui_control) * typed_config->max_control_count);
    typed_state->inactive_controls = (void*)((u8*)typed_state->active_controls + active_array_requirement);
    kzero_memory(typed_state->inactive_controls, sizeof(sui_control) * typed_config->max_control_count);

    // sui_base_control_create("__ROOT__",&typed_state->root);

    KTRACE("Initialized standard UI system.");

    return true;
}

void standard_ui_system_shutdown(void* state) {
    if (state) {
        standard_ui_state* typed_state = (standard_ui_state*)state;
        for (u32 i = 0; i < typed_state->inactive_control_count; i++) {
            sui_control* c = typed_state->inactive_controls[i];
            c->unload(c);
            c->destroy(c);
        }

        for (u32 i = 0; i < typed_state->active_control_count; i++) {
            sui_control* c = typed_state->active_controls[i];
            c->unload(c);
            c->destroy(c);
        }
    }
}

b8 standard_ui_system_update(void* state, struct frame_data* p_frame_data) {
    if (!state) {
        return false;
    }
    // 更新正在激活的组件
    standard_ui_state* typed_state = (standard_ui_state*)state;
    for (u32 i = 0; i < typed_state->active_control_count; i++) {
        sui_control* c = typed_state->active_controls[i];
        c->update(c, p_frame_data);
    }

    return true;
}

b8 standard_ui_system_render(void* state, sui_control* root, struct frame_data* p_frame_data, standard_ui_render_data* render_data) {
    if (!state) {
        return false;
    }

    standard_ui_state* typed_state = (standard_ui_state*)state;
    // 图集
    render_data->ui_atlas = &typed_state->ui_atlas;

    if (!root) {
        root = &typed_state->root;
    }

    if (root->render) {
        // render函数指针
        if (!root->render(root, p_frame_data, render_data)) {
            KERROR("Root element failed to render. See logs for more details");
            return false;
        }
    }

    if (root->children) {
        u32 length = darray_length(root->children);
        for (u32 i = 0; i < length; ++i) {
            sui_control* c = root->children[i];
            if (!c->is_visible) {
                continue;
            }
            if (!standard_ui_system_render(state, c, p_frame_data, render_data)) {
                KERROR("Child element failed to render. See logs for more details");
                return false;
            }
        }
    }
    return true;
}

b8 standard_ui_system_update_active(void* state, sui_control* control) {
    if (!state) {
        return false;
    }
    standard_ui_state* typed_state = (standard_ui_state*)state;
    u32* src_limit = control->is_active ? &typed_state->inactive_control_count : &typed_state->active_control_count;
    u32* dst_limt = control->is_active ? &typed_state->active_control_count : &typed_state->inactive_control_count;
    sui_control** src_array = control->is_active ? typed_state->inactive_controls : typed_state->active_controls;
    sui_control** dst_array = control->is_active ? typed_state->active_controls : typed_state->inactive_controls;
    for (u32 i = 0; i < *src_limit; i++) {
        if (src_array[i] == control) {
            sui_control* c = src_array[i];
            // 放入激活数组
            dst_array[*dst_limt] = c;
            (*dst_limt)++;

            // Copy the reset of the array inward.
            kcopy_memory(((u8*)src_array) + (i * sizeof(sui_control*)), ((u8*)src_array) + ((i + 1) * sizeof(sui_control*)), sizeof(sui_control*) * ((*src_limit) - i));
            src_array[*src_limit] = 0;
            (*src_limit)--;
            return true;
        }
    }

    KERROR("Unable to find control to update active on,maybe control is not registered.");
    return false;
}

b8 standard_ui_system_register_control(void* state, sui_control* control) {
    if (!state) {
        return false;
    }

    standard_ui_state* typed_state = (standard_ui_state*)state;
    if (typed_state->total_control_count >= typed_state->config.max_control_count) {
        KERROR("Unable to find free space to register control.Registration failed.");
        return false;
    }

    typed_state->total_control_count++;
    // Found available space.put it there
    typed_state->inactive_controls[typed_state->inactive_control_count] = control;
    typed_state->inactive_control_count++;
    return true;
}

b8 sui_base_control_create(const char* name, struct sui_control* out_control) {
    if (!out_control) {
        return false;
    }

    // Set all controls to visible by default.
    out_control->is_visible = true;

    // Assign function pointers.
    out_control->destroy = sui_base_control_destroy;
    out_control->load = sui_base_control_load;
    out_control->unload = sui_base_control_unload;
    out_control->update = sui_base_control_update;
    out_control->render = sui_base_control_render;

    out_control->name = string_duplicate(name);
    // out_control->id
    out_control->xform = transform_create();
    return true;
}

void sui_base_control_destroy(struct sui_control* self) {
    if (self) {
        // TODO:recurse children/unparent?
        if (self->internal_data && self->internal_data_size) {
            kfree(self->internal_data, self->internal_data_size, MEMORY_TAG_UI);
        }
        if (self->name) {
            string_free(self->name);
        }
        kzero_memory(self, sizeof(sui_control));
    }
}

b8 sui_base_control_load(struct sui_control* self) {
    if (!self) {
        return false;
    }

    return true;
}

void sui_base_control_unload(struct sui_control* self) {
    if (!self) {
        //
    }
}

b8 sui_base_control_update(struct sui_control* self, struct frame_data* p_frame_data) {
    if (!self) {
        return false;
    }
    return true;
}

b8 sui_base_control_render(struct sui_control* self, struct frame_data* p_frame_data, standard_ui_render_data* render_data) {
    if (!self) {
        return false;
    }
    return true;
}

void sui_control_position_set(struct sui_control* self, vec3 position) {
    transform_position_set(&self->xform, position);
}

vec3 sui_control_position_get(struct sui_control* self) {
    return transform_position_get(&self->xform);
}

typedef struct sui_panel_internal_data {
    vec4 rect;
    vec4 colour;
    geometry g;
} sui_panel_internal_data;

b8 sui_panel_control_create(const char* name, struct sui_control* out_control) {
    if (!sui_base_control_create(name, out_control)) {
        return false;
    }

    out_control->internal_data_size = sizeof(sui_panel_internal_data);
    out_control->internal_data = kallocate(out_control->internal_data_size, MEMORY_TAG_UI);
    sui_panel_internal_data* typed_data = out_control->internal_data;

    // Reasonable defaults.
    typed_data->rect = vec4_create(0, 0, 10, 10);
    typed_data->colour = vec4_one();

    // Assign function pointers.
    out_control->destroy = sui_panel_control_destroy;
    out_control->load = sui_panel_control_load;
    out_control->unload = sui_panel_control_unload;
    out_control->update = sui_panel_control_update;
    out_control->render = sui_panel_control_render;

    out_control->name = string_duplicate(name);

    return true;
}

void sui_panel_control_destroy(struct sui_control* self) {
    sui_base_control_destroy(self);
}

b8 sui_panel_control_load(struct sui_control* self) {
    if (!sui_base_control_load(self)) {
        return false;
    }

    return true;
}

void sui_panel_control_unload(struct sui_control* self) {
}

b8 sui_panel_control_update(struct sui_control* self, struct frame_data* p_frame_data) {
    if (!sui_base_control_update(self, p_frame_data)) {
        return false;
    }
    return true;
}

b8 sui_panel_control_render(struct sui_control* self, struct frame_data* p_frame_data, standard_ui_render_data* render_data) {
    if (!sui_base_control_render(self, p_frame_data, render_data)) {
        return false;
    }

    return true;
}