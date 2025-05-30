#include "logger.h"
#include "assert.h"
#include "kstring.h"
#include "kmemory.h"
#include "platform/platform.h"
#include "platform/filesystem.h"

#include <stdarg.h>

// A console hook function pointer.
static PFN_console_write console_hook = 0;

//函数指针 也指钩子
void logger_console_write_hook_set(PFN_console_write hook) {
    console_hook = hook;
}

// typedef struct logger_system_state {
//     file_handle log_file_handle;
//     ;
// } logger_system_state;

// static logger_system_state* state_ptr;

// static void append_to_log_file(const char* message) {
//     if (state_ptr && state_ptr->log_file_handle.is_valid) {
//         // Since the message already contains a '\n', just write the bytes directly.
//         u64 length = string_length(message);
//         u64 written = 0;
//         if (!filesystem_write(&state_ptr->log_file_handle, length, message, &written)) {
//             platform_console_write_error("ERROR writing to console.log.", LOG_LEVEL_ERROR);
//         }
//     }
// }

// b8 logging_initialize(u64* memory_requirement, void* state, void* config) {
//     *memory_requirement = sizeof(logger_system_state);
//     if (state == 0) {
//         return true;
//     }

//     state_ptr = state;

//     // Create new/wipe existing log file ,the open it
//     if (!filesystem_open("console.log", FILE_MODE_WRITE, false, &state_ptr->log_file_handle)) {
//         platform_console_write_error("ERROR: Unable to open console.log for writing.", LOG_LEVEL_ERROR);
//         return false;
//     }

//     return true;
// }

// void logging_shutdown(void* state) {
//     state_ptr = 0;
//     // TODO: cleanup logging/write queued entries
// }

// 可变参数同样是char*
KAPI void log_output(log_level level, const char* message, ...) {
    const char* level_strings[6] = {"[FATAL]:", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};

    // Technically imposes a 32k character limit on a single log entry, but...
    // DON'T DO THAT!
    char out_message[32000];
    kzero_memory(out_message, sizeof(out_message));

    string_format(out_message, "%s%s\n", level_strings[level], out_message);
    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    string_format_v(out_message, message, arg_ptr);
    va_end(arg_ptr);

    // If the console hook is defined, make sure to forward messages to it, and it will pass along to consumers.
    // Otherwise the platform layer will be used directly.
    if (console_hook) {
        console_hook(level,out_message);
    } else {
        platform_console_write(0,level,out_message);
    }
}

KAPI void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
    log_output(LOG_LEVEL_FATAL, "Assertion Failure:%s,message:'%s',in fail:%s,line:%d\n", expression, message, file, line);
}
