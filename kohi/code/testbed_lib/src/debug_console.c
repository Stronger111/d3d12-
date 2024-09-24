#include "debug_console.h"

#include <core/console.h>
#include <core/kmemory.h>
#include <core/kstring.h>
#include <containers/darray.h>
#include <core/event.h>
#include <core/input.h>

b8 debug_console_consumer_write(void* inst, log_level level, const char* message) {
    debug_console_state* state = (debug_console_state*)inst;
    if (state) {
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

static b8 debug_console_on_key(u16 code, void* sender, void* listener_inst, event_context context) {
    debug_console_state* state = (debug_console_state*)listener_inst;
    if (!state->visible) {
        return false;
    }
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 key_code = context.data.u16[0];
        b8 shift_held = input_is_key_down(KEY_LSHIFT) || input_is_key_down(KEY_RSHIFT) || input_is_key_down(KEY_SHIFT);

        // 判断敲入参数
        if (key_code == KEY_ENTER) {
            u32 len = string_length(state->entry_control.text);
            if (len > 0) {
                // 保存命令历史列表
                command_history_entry entry;
                entry.command = string_duplicate(state->entry_control.text);
                darray_push(state->history, entry);

                // 执行命令并且清理文本
                if (!console_execute_command(state->entry_control.text)) {
                    // TODO:处理错误？
                }
                // 清理文本
                ui_text_set_text(&state->entry_control, "");
            }
        } else if (key_code == KEY_BACKSPACE) {
            u32 len = string_length(state->entry_control.text);
            if (len > 0) {
                char* str = string_duplicate(state->entry_control.text);
                str[len - 1] = 0;
                ui_text_set_text(&state->entry_control, str);
                kfree(str, len + 1, MEMORY_TAG_STRING);
            }
        } else {
            // 用 A-Z 和 0-9
            char char_code = key_code;
            if ((key_code >= KEY_A && key_code <= KEY_Z)) {
                // TODO:检查大写
                if (!shift_held) {
                    char_code = key_code + 32;
                }
            } else if ((key_code >= KEY_0 && key_code <= KEY_9)) {
                if (shift_held) {
                    // NOTE: this handles US standard keyboard layouts.
                    // Will need to handle other layouts as well.
                    switch (key_code) {
                        case KEY_0:
                            char_code = ')';
                            break;
                        case KEY_1:
                            char_code = '!';
                            break;
                        case KEY_2:
                            char_code = '@';
                            break;
                        case KEY_3:
                            char_code = '#';
                            break;
                        case KEY_4:
                            char_code = '$';
                            break;
                        case KEY_5:
                            char_code = '%';
                            break;
                        case KEY_6:
                            char_code = '^';
                            break;
                        case KEY_7:
                            char_code = '&';
                            break;
                        case KEY_8:
                            char_code = '*';
                            break;
                        case KEY_9:
                            char_code = '(';
                            break;
                    }
                }
            } else {
                switch (key_code) {
                    case KEY_SPACE:
                        char_code = key_code;
                        break;
                    case KEY_MINUS:
                        char_code = shift_held ? '_' : '-';
                        break;
                    case KEY_EQUAL:
                        char_code = shift_held ? '+' : '=';
                        break;
                    default:
                        // Not valid for entry, use 0
                        char_code = 0;
                        break;
                }
            }
            if (char_code != 0) {
                u32 len = string_length(state->entry_control.text);
                char* new_text = kallocate(len + 2, MEMORY_TAG_STRING);
                string_format(new_text, "%s%c", state->entry_control.text, char_code);
                ui_text_set_text(&state->entry_control, new_text);
                kfree(new_text, len + 1, MEMORY_TAG_STRING);
            }
        }
        // TODO:存储历史命令,可以按上下键导航
    }
    return false;
}

void debug_console_create(debug_console_state* out_console_state) {
    if (out_console_state) {
        out_console_state->line_display_count = 10;
        out_console_state->line_offset = 0;
        out_console_state->lines = darray_create(char*);
        out_console_state->visible = false;
        out_console_state->history = darray_create(command_history_entry);
        out_console_state->history_offset = 0;

        // NOTE:根据要显示的行数更新文本 并且距底部偏移的行数,UI Text 对象目前用于显示
        // 可以在分开的通道中糟糕的颜色。 不考虑自动换行
        // NOTE:另外应该考虑裁切矩形和新行

        // 注册一个控制台消费者
        console_register_consumer(out_console_state, debug_console_consumer_write, &out_console_state->console_consumer_id);
        // Register for key events.
        event_register(EVENT_CODE_KEY_PRESSED, out_console_state, debug_console_on_key);
        event_register(EVENT_CODE_KEY_RELEASED, out_console_state, debug_console_on_key);
    }
}

b8 debug_console_load(debug_console_state* state) {
    if (!state) {
        KFATAL("debug_console_load() called before console was initialized!");
        return false;
    }

    // Create a ui text control for rendering.
    if (!ui_text_create(UI_TEXT_TYPE_SYSTEM, "Noto Sans CJK JP", 31, "", &state->text_control)) {
        KFATAL("Unable to create text control for debug console.");
        return false;
    }

    ui_text_set_position(&state->text_control, (vec3){3.0f, 30.0f, 0.0f});

    // Create another ui text control for rendering typed text.
    if (!ui_text_create(UI_TEXT_TYPE_SYSTEM, "Noto Sans CJK JP", 31, "", &state->entry_control)) {
        KFATAL("Unable to create entry text control for debug console.");
        return false;
    }

    ui_text_set_position(&state->entry_control, (vec3){3.0f, 30.0f + (31.0f * state->line_display_count), 0.0f});

    return true;
}

void debug_console_unload(debug_console_state* state) {
    if (state) {
        ui_text_destroy(&state->text_control);
        ui_text_destroy(&state->entry_control);
    }
}

void debug_console_update(debug_console_state* state) {
    if (state && state->dirty) {
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
        ui_text_set_text(&state->text_control, buffer);
        state->dirty = false;
    }
}

void debug_console_on_lib_load(debug_console_state* state, b8 update_consumer) {
    if (update_consumer) {
        event_register(EVENT_CODE_KEY_PRESSED, state, debug_console_on_key);
        event_register(EVENT_CODE_KEY_RELEASED, state, debug_console_on_key);
        console_update_consumer(state->console_consumer_id, state, debug_console_consumer_write);
    }
}


void debug_console_on_lib_unload(debug_console_state* state) {
    event_unregister(EVENT_CODE_KEY_PRESSED, state, debug_console_on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, state, debug_console_on_key);
    console_update_consumer(state->console_consumer_id, 0, 0);
}

ui_text* debug_console_get_text(debug_console_state* state) {
    if (state) {
        return &state->text_control;
    }
    return 0;
}

ui_text* debug_console_get_entry_text(debug_console_state* state) {
    if (state) {
        return &state->entry_control;
    }
    return 0;
}

b8 debug_console_visible(debug_console_state* state) {
    if (!state) {
        return false;
    }

    return state->visible;
}

void debug_console_visible_set(debug_console_state* state,b8 visible) {
    if (state) {
        state->visible = visible;
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
        if(state->line_offset == 0)
        {
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
        u32 length = darray_length(state->history);
        if (length > 0) {
            state->history_offset = KMIN(state->history_offset++, length - 1);
            ui_text_set_text(&state->entry_control, state->history[length - state->history_offset - 1].command);
        }
    }
}

void debug_console_history_forward(debug_console_state* state) {
    if (state) {
        u32 length = darray_length(state->history);
        if (length > 0) {
            state->history_offset = KMAX(state->history_offset--, 0);
            ui_text_set_text(&state->entry_control, state->history[length - state->history_offset - 1].command);
        }
    }
}