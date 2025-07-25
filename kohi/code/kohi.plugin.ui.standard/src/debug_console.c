#include "debug_console.h"

#include <containers/darray.h>
#include <core/console.h>
#include <core/event.h>
#include <core/input.h>
#include <memory/kmemory.h>
#include <resources/resource_types.h>
#include <strings/kstring.h>

#include "controls/sui_label.h"
#include "controls/sui_panel.h"
#include "controls/sui_textbox.h"
#include "standard_ui_system.h"

static void debug_console_entry_box_on_key(standard_ui_state* state,sui_control* self, sui_keyboard_event evt);

b8 debug_console_consumer_write(void* inst, log_level level, const char* message) {
    debug_console_state* state = (debug_console_state*)inst;
    if (state) {
        // Not necessarily a failure, but move on if not loaded.
        if (!state->loaded) {
            return true;
        }
        // For high-priority error/fatal messages, don't bother with splitting,
        // just output them because something truly terrible could prevent this
        // split from happening.
        if (level <= LOG_LEVEL_ERROR) {
            // NOTE: Trim the string to get rid of the newline appended at the console level
            darray_push(state->lines, string_trim(string_duplicate(message)));
            state->dirty = true;
            return true;
        }
        // 创建一个字符串的新副本,然后进行拆分
        // 通过换行符使每个行都算作新行
        // 注意：故意缺少清理字符串操作
        // 在这里，因为字符串需要继续存在，这样它们才能
        // 可以通过该调试控制台访问。一般情况下是清理
        // 通过 string_cleanup_split_array 是有道理的。
        char** split_message = darray_create(char*);
        u32 count = string_split(message, '\n', &split_message, true, false);
        // 把每一个新行放入数组中
        for (u32 i = 0; i < count; ++i) {
            darray_push(state->lines, split_message[i]);
        }

        // 清理临时数组
        darray_destroy(split_message);
        state->dirty = true;
    }
    return true;
}

static b8 debug_console_on_resize(u16 code, void* sender, void* listener_inst, event_context context) {
    u16 width = context.data.u16[0];
    /* u16 height = context.data.u16[1]; */
    debug_console_state* state = listener_inst;
    vec2 size = sui_panel_size(state->sui_state,&state->bg_panel);
    sui_panel_control_resize(state->sui_state,&state->bg_panel, (vec2){width, size.y});
    return false;
}

void debug_console_create(standard_ui_state* sui_state, debug_console_state* out_console_state) {
    if (out_console_state) {
        out_console_state->line_display_count = 10;
        out_console_state->line_offset = 0;
        out_console_state->lines = darray_create(char*);
        out_console_state->visible = false;
        out_console_state->history = darray_create(command_history_entry);
        out_console_state->history_offset = -1;
        out_console_state->loaded = false;
        out_console_state->sui_state=sui_state;

        // NOTE:根据要显示的行数更新文本 并且距底部偏移的行数,UI Text 对象目前用于显示
        // 可以在分开的通道中糟糕的颜色。 不考虑自动换行
        // NOTE:另外应该考虑裁切矩形和新行

        // 注册一个控制台消费者
        console_consumer_register(out_console_state, debug_console_consumer_write, &out_console_state->console_consumer_id);
        // Register for key events.
        event_register(EVENT_CODE_RESIZED, out_console_state, debug_console_on_resize);
    }
}

b8 debug_console_load(debug_console_state* state) {
    if (!state) {
        KFATAL("debug_console_load() called before console was initialized!");
        return false;
    }

    u16 font_size = 31;
    f32 height = 50.0f + (font_size * state->line_display_count + 1);  // Account for padding and textbox at the bottom.

    if (!sui_panel_control_create(state->sui_state,"debug_console_bg_panel", (vec2){1280.0f, height}, (vec4){0.0f, 0.0f, 0.0f, 0.75f}, &state->bg_panel)) {
        KERROR("Failed to create background panel.");
    } else {
        if (!sui_panel_control_load(state->sui_state,&state->bg_panel)) {
            KERROR("Failed to load background panel.");
        } else {
            /* transform_translate(&state->bg_panel.xform, (vec3){500, 100}); */
            if (!standard_ui_system_register_control(state->sui_state, &state->bg_panel)) {
                KERROR("Unable to register control.");
            } else {
                if (!standard_ui_system_control_add_child(state->sui_state, 0, &state->bg_panel)) {
                    KERROR("Failed to parent background panel.");
                } else {
                    state->bg_panel.is_active = true;
                    state->bg_panel.is_visible = false;
                    if (!standard_ui_system_update_active(state->sui_state, &state->bg_panel)) {
                        KERROR("Unable to update active state.");
                    }
                }
            }
        }
    }

    // Create a ui text control for rendering.
    if (!sui_label_control_create(state->sui_state,"debug_console_log_text", FONT_TYPE_SYSTEM, "Noto Sans CJK JP", font_size, "", &state->text_control)) {
        KFATAL("Unable to create text control for debug console.");
        return false;
    } else {
        if (!sui_panel_control_load(state->sui_state,&state->text_control)) {
            KERROR("Failed to load text control.");
        } else {
            if (!standard_ui_system_register_control(state->sui_state, &state->text_control)) {
                KERROR("Unable to register control");
            } else {
                if (!standard_ui_system_control_add_child(state->sui_state, &state->bg_panel, &state->text_control)) {
                    KERROR("Failed to parent background panel.");
                } else {
                    state->text_control.is_active = true;
                    if (!standard_ui_system_update_active(state->sui_state, &state->text_control)) {
                        KERROR("Unable to update active state.");
                    }
                }
            }
        }
    }

    sui_control_position_set(state->sui_state,&state->text_control, (vec3){3.0f, font_size, 0.0f});

    // Create another ui text control for rendering typed text.

    // new one
    if (!sui_textbox_control_create(state->sui_state,"debug_console_entry_textbox", FONT_TYPE_SYSTEM, "Noto Sans CJK JP", font_size, "", &state->entry_textbox)) {
        KFATAL("Unable to create textbox control for debug console.");
        return false;
    } else {
        if (!state->entry_textbox.load(state->sui_state,&state->entry_textbox)) {
            KERROR("Failed to load entry textbox for debug console.");
        } else {
            state->entry_textbox.user_data = state;
            state->entry_textbox.user_data_size = sizeof(debug_console_state*);
            state->entry_textbox.on_key=debug_console_entry_box_on_key;
            if (!standard_ui_system_register_control(state->sui_state, &state->entry_textbox)) {
                KERROR("Unable to register control.");
            } else {
                if (!standard_ui_system_control_add_child(state->sui_state, &state->bg_panel, &state->entry_textbox)) {
                    KERROR("Failed to parent textbox control to background panel of debug console.");
                } else {
                    state->entry_textbox.is_active = true;
                    if (!standard_ui_system_update_active(state->sui_state, &state->entry_textbox)) {
                        KERROR("Unable to update active state.");
                    }
                }
            }
        }
    }

    // HACK:This is definitely not the best way to figure out the hight of the above text control.
    sui_control_position_set(state->sui_state,&state->entry_textbox, (vec3){3.0f, 10.0f + (font_size * state->line_display_count), 0.0f});

    state->loaded = true;

    return true;
}

void debug_console_unload(debug_console_state* state) {
    if (state) {
        state->loaded = false;
    }
}

void debug_console_update(debug_console_state* state) {
    if (state && state->loaded && state->dirty) {
        u32 line_count = darray_length(state->lines);
        u32 max_lines = KMIN(state->line_display_count, KMAX(line_count, state->line_display_count));

        // 首先计算最小的行,还要考虑行偏移
        u32 min_line = KMAX(line_count - max_lines - state->line_offset, 0);
        u32 max_line = min_line + max_lines - 1;

        // 希望足够大来处理更多的事情
        char buffer[16384];
        kzero_memory(buffer, sizeof(char) * 16384);
        u32 buffer_pos = 0;
        for (u32 i = min_line; i <= max_line; ++i) {
            // TODO:对于消息类型插入颜色代码

            const char* line = state->lines[i];
            u32 line_length = string_length(line);
            for (u32 c = 0; c < line_length; c++, buffer_pos++) {
                buffer[buffer_pos] = line[c];
            }
            // 添加一个新行
            buffer[buffer_pos] = '\n';
            buffer_pos++;
        }

        // 确认字符串结尾为空
        buffer[buffer_pos] = '\0';

        // 构建字符串后,设置文本
        sui_label_text_set(state->sui_state,&state->text_control, buffer);
        state->dirty = false;
    }
}

static void debug_console_entry_box_on_key(standard_ui_state* state,sui_control* self, sui_keyboard_event evt) {
    if (evt.type == SUI_KEYBOARD_EVENT_TYPE_PRESS) {
        u16 key_code = evt.key;
        /* b8 shift_held = input_is_key_down(KEY_LSHIFT) || input_is_key_down(KEY_RSHIFT) || input_is_key_down(KEY_SHIFT); */
        // 回车键
        if (key_code == KEY_ENTER) {
            debug_console_state* state=self->internal_data;
            const char* entry_control_text = sui_textbox_text_get(state->sui_state,self);
            u32 len = string_length(entry_control_text);
            if (len > 0) {
                // Keep the command in the history list.
                command_history_entry entry = {0};
                entry.command = string_duplicate(entry_control_text);
                if (entry.command) {
                    darray_push(((debug_console_state*)self->user_data)->history, entry);

                    // Execute the command and clear the text.
                    if (!console_command_execute(entry_control_text)) {
                        // TODO:handle error!
                    }
                }
                // clear the text.
                sui_textbox_text_set(state->sui_state,self, "");
            }
        }
    }
}

void debug_console_on_lib_load(debug_console_state* state, b8 update_consumer) {
    if (update_consumer) {
        state->entry_textbox.on_key = debug_console_entry_box_on_key;
        event_register(EVENT_CODE_RESIZED, state, debug_console_on_resize);
        console_consumer_update(state->console_consumer_id, state, debug_console_consumer_write);
    }
}

void debug_console_on_lib_unload(debug_console_state* state) {
    state->entry_textbox.on_key = 0;
    event_unregister(EVENT_CODE_RESIZED, state, debug_console_on_resize);
    console_consumer_update(state->console_consumer_id, 0, 0);
}

sui_control* debug_console_get_text(debug_console_state* state) {
    if (state) {
        return &state->text_control;
    }
    return 0;
}

sui_control* debug_console_get_entry_text(debug_console_state* state) {
    if (state) {
        return &state->entry_textbox;
    }
    return 0;
}

b8 debug_console_visible(debug_console_state* state) {
    if (!state) {
        return false;
    }

    return state->visible;
}

void debug_console_visible_set(debug_console_state* state, b8 visible) {
    if (state) {
        state->visible = visible;
        // 显示背景
        state->bg_panel.is_visible = visible;
        standard_ui_system_focus_control(state->sui_state, visible ? &state->entry_textbox : 0);
        input_key_repeats_enable(visible);
    }
}

void debug_console_move_up(debug_console_state* state) {
    if (state) {
        state->dirty = true;
        u32 line_count = darray_length(state->lines);
        // 不需要尝试偏移,只需要重置并且启动
        if (line_count <= state->line_display_count) {
            state->line_offset = 0;
            return;
        }
        state->line_offset++;
        state->line_offset = KMIN(state->line_offset, line_count - state->line_display_count);
    }
}

void debug_console_move_down(debug_console_state* state) {
    if (state) {
        if (state->line_offset == 0) {
            return;
        }
        state->dirty = true;
        u32 line_count = darray_length(state->lines);
        // Don't bother with trying an offset, just reset and boot out.
        if (line_count <= state->line_display_count) {
            state->line_offset = 0;
            return;
        }

        state->line_offset--;
        state->line_offset = KMAX(state->line_offset, 0);
    }
}

void debug_console_move_to_top(debug_console_state* state) {
    if (state) {
        state->dirty = true;
        u32 line_count = darray_length(state->lines);
        // Don't bother with trying an offset, just reset and boot out.
        if (line_count <= state->line_display_count) {
            state->line_offset = 0;
            return;
        }

        state->line_offset = line_count - state->line_display_count;
    }
}

void debug_console_move_to_bottom(debug_console_state* state) {
    if (state) {
        state->dirty = true;
        state->line_offset = 0;
    }
}

void debug_console_history_back(debug_console_state* state) {
    if (state) {
        i32 length = darray_length(state->history);
        if (length > 0) {
            state->history_offset = KMIN(state->history_offset + 1, length - 1);
            i32 idx = length - state->history_offset - 1;
            sui_textbox_text_set(state->sui_state,&state->entry_textbox, state->history[idx].command);
        }
    }
}

void debug_console_history_forward(debug_console_state* state) {
    if (state) {
        i32 length = darray_length(state->history);
        if (length > 0) {
            state->history_offset = KMAX(state->history_offset - 1, -1);
            i32 idx = length - state->history_offset - 1;
            sui_textbox_text_set(state->sui_state,&state->entry_textbox, state->history_offset == -1 ? "" : state->history[idx].command);
        }
    }
}