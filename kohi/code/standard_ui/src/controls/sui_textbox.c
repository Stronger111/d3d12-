#include "sui_textbox.h"

#include <containers/darray.h>
#include <core/event.h>
#include <core/kmemory.h>
#include <core/kstring.h>
#include <core/logger.h>
#include <core/systems_manager.h>
#include <math/kmath.h>
#include <math/transform.h>
#include <renderer/renderer_frontend.h>
#include <systems/geometry_system.h>
#include <systems/shader_system.h>

#include "controls/sui_panel.h"
#include "core/input.h"
#include "defines.h"
#include "math/geometry_utils.h"
#include "resources/resource_types.h"
#include "standard_ui_system.h"
#include "sui_label.h"
#include "systems/font_system.h"

/*
static b8 sui_textbox_on_key(u16 code, void* sender, void* listener_inst, event_context context);

static f32 sui_textbox_calculate_offset(u32 string_pos, const char* full_string, font_data* font) {
    if (string_pos == 0) {
        return 0;
    }

    char* mid_target = string_duplicate(full_string);
    u32 original_length = string_length(mid_target);
    string_mid(mid_target, full_string, 0, string_pos);

    vec2 size = font_system_measure_string(font, mid_target);

    // Make sure to cleanup the string.
    kfree(mid_target, sizeof(char) * original_length + 1, MEMORY_TAG_STRING);

    // Use the x-axis of the mesurement to place the cursor.
    return size.x;
}

static void sui_textbox_update_highlight_box(sui_control* self) {
    sui_textbox_internal_data* typed_data = self->internal_data;

    if (typed_data->highlight_range.size == 0) {
        typed_data->highlight_box.is_visible = false;
        return;
    }

    typed_data->highlight_box.is_visible = true;
}

b8 sui_textbox_control_create(const char* name, font_type type, const char* font_name, u16 font_size, const char* text, sui_control* out_control) {
    return true;
}
*/