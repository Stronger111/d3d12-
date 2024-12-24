#pragma once
#include "defines.h"

typedef struct platform_system_config {
    /** @brief application_name The name of the application. */
    const char* application_name;
    /** @brief x The initial x position of the main window. */
    i32 x;
    /** @brief y The initial y position of the main window.*/
    i32 y;
    /** @brief width The initial width of the main window. */
    i32 width;
    /** @brief height The initial height of the main window. */
    i32 height;
} platform_system_config;

typedef struct dynamic_library_function {
    const char* name;
    void* pfn;
} dynamic_library_function;

typedef struct dynamic_library {
    const char* name;
    const char* filename;
    u64 internal_data_size;
    void* internal_data;
    u32 watch_id;

    // darray
    dynamic_library_function* functions;
} dynamic_library;

typedef enum platform_error_code {
    PLATFORM_ERROR_SUCCESS = 0,
    PLATFORM_ERROR_UNKNOWN = 1,
    PLATFORM_ERROR_FILE_NOT_FOUND = 2,
    PLATFORM_ERROR_FILE_LOCKED = 3,
    PLATFORM_ERROR_EXISTS = 4
} platform_error_code;

/**
 * @brief Performs startup routines within the platform layer. Should be called twice,
 * once to obtain the memory requirement (with state=0), then a second time passing
 * an allocated block of memory to state.
 *
 * @param memory_requirement A pointer to hold the memory requirement in bytes.
 * @param state A pointer to a block of memory to hold state. If obtaining memory requirement only, pass 0.
 * @param config A pointer to a configuration platform_system_config structure required by this system.
 * @return True on success; otherwise false.
 */
b8 platform_system_startup(
    u64* memory_requirement,
    void* state,
    void* config);

void platform_system_shutdown(void* plat_state);
/**
 * @brief Performs any platform-specific message pumping that is required
 * for windowing, etc.
 *
 * @return True on success; otherwise false.
 */
b8 platform_pump_messages(void);
// aligned 内存对齐 b8 8bit
void* platform_allocate(u64 size, b8 aligned);
void platform_free(void* block, b8 aligned);
void* platform_zero_memory(void* block, u64 size);
void* platform_copy_memory(void* dest, const void* source, u64 size);
void* platform_set_memory(void* dest, i32 value, u64 size);

void platform_console_write(const char* message, u8 colour);
void platform_console_write_error(const char* message, u8 colour);

f64 platform_get_absolute_time(void);

KAPI void platform_sleep(u64 ms);

/**
 * @brief Obtains the number of logical processor cores.
 *
 * @return The number of logical processor cores.
 */
i32 platform_get_processor_count(void);

/**
 * @brief Obtains the required memory amount for platform-specific handle data,
 * and optionally obtains a copy of that data. Call twice, once with memory=0
 * to obtain size, then a second time where memory = allocated block.
 *
 * @param out_size A pointer to hold the memory requirement.
 * @param memory Allocated block of memory.
 */
KAPI void platform_get_handle_info(u64* out_size, void* memory);

/**
 * @brief Returns the device pixel ratio of the main window.
 */
KAPI f32 platform_device_pixel_ratio(void);

/**
 * @brief Loads a dynamic library.
 *
 * @param name The name of the library file, *excluding* the extension. Required.
 * @param out_library A pointer to hold the loaded library. Required.
 * @return True on success; otherwise false.
 */
KAPI b8 platform_dynamic_library_load(const char* name, dynamic_library* out_library);

/**
 * @brief Unloads the given dynamic library.
 *
 * @param library A pointer to the loaded library. Required.
 * @return True on success; otherwise false.
 */
KAPI b8 platform_dynamic_library_unload(dynamic_library* library);

/**
 * @brief Loads an exported function of the given name from the provided loaded library.
 *
 * @param name The function name to be loaded.
 * @param library A pointer to the library to load the function from.
 * @return True on success; otherwise false.
 */
KAPI b8 platform_dynamic_library_load_function(const char* name, dynamic_library* library);


/**
 * @brief Returns the file extension for the current platform.
 */
KAPI const char* platform_dynamic_library_extension(void);

/**
 * @brief Returns a file prefix for libraries for the current platform.
 */
KAPI const char* platform_dynamic_library_prefix(void);


/**
 * @brief Copies file at source to destination, optionally overwriting.
 * 
 * @param source The source file path.
 * @param dest The destination file path.
 * @param overwrite_if_exists Indicates if the file should be overwritten if it exists.
 * @return An error code indicating success or failure.
 */
KAPI platform_error_code platform_copy_file(const char* source, const char* dest, b8 overwrite_if_exists);

/**
 * @brief Watch a file at the given path.
 *
 * @param file_path The file path. Required.
 * @param out_watch_id A pointer to hold the watch identifier.
 * @return True on success; otherwise false.
 */
KAPI b8 platform_watch_file(const char* file_path, u32* out_watch_id);

/**
 * @brief Stops watching the file with the given watch identifier.
 *
 * @param watch_id The watch identifier
 * @return True on success; otherwise false.
 */
KAPI b8 platform_unwatch_file(u32 watch_id);
