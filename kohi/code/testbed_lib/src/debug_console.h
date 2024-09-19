#pragma once

#include "defines.h"

#include <resources/ui_text.h>

typedef struct command_history_entry {
    const char* command;
} command_history_entry;

// TODO(travis): statically-defined state for now.
typedef struct debug_console_state {
    u8 console_consumer_id;
    // 一次显示的行数
    i32 line_display_count;
    // 距列表底部的行数
    i32 line_offset;
    // 列表
    char** lines;

    // 命令历史
    command_history_entry* history;
    i32 history_offset;

    b8 dirty;
    b8 visible;

    ui_text text_control;
    ui_text entry_control;
} debug_console_state;

void debug_console_create(debug_console_state* out_console_state);

b8 debug_console_load(debug_console_state* state);
void debug_console_unload(debug_console_state* state);
void debug_console_update(debug_console_state* state);

void debug_console_on_lib_load(debug_console_state* state, b8 update_consumer);
void debug_console_on_lib_unload(debug_console_state* state);

struct ui_text* debug_console_get_text(debug_console_state* state);
struct ui_text* debug_console_get_entry_text(debug_console_state* state);

b8 debug_console_visible(debug_console_state* state);
void debug_console_visible_set(debug_console_state* state, b8 visible);

void debug_console_move_up(debug_console_state* state);
void debug_console_move_down(debug_console_state* state);
void debug_console_move_to_top(debug_console_state* state);
void debug_console_move_to_bottom(debug_console_state* state);

void debug_console_history_back(debug_console_state* state);
void debug_console_history_forward(debug_console_state* state);


