#pragma once

#include "defines.h"

// Holds a handle to a file
typedef struct file_handle {
    // Opaque handle to internal file
    void* handle;
    b8 is_valid;
} file_handle;

typedef enum file_modes {
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2,
} file_modes;

/**
 * @brief If func returns false, closes the provided file handle and logs an error.
 * Also returns false, so the calling function must return a boolean. Calling file
 * must #include "logger.h".
 * @param func The function whose result is a boolean.
 * @param handle The file handle to be closed on a false function result.
 * @returns False if the provided function fails.
 */
#define CLOSE_IF_FAILED(func, handle)     \
    if (!func) {                          \
        KERROR("File operation failed."); \
        filesystem_close(handle);         \
        return false;                     \
    }

/**
 * Checks if a file with the given path exists
 * @param path The path of the file to be checked
 * @return True if exists. otherwise false.
 */
KAPI b8 filesystem_exists(const char* path);

/**
 * @brief
 *  Attempt to open file located at path
 * @param path The Path of the file to be opened
 * @param mode Mode flags for the file when opened (read/write). See file_modes enum in filesystem.h
 * @param binary Indicates if the file should be opened in binary mode.
 * @param out_handle A Pointer to a file_handle structure which holds the handle information.
 * @return True if opened successfully; otherwise false.
 */
KAPI b8 filesystem_open(const char* path, file_modes mode, b8 binary, file_handle* out_handle);

/**
 * @brief Closes the provided handle to a file
 *
 * @param handle  A Pointer to a file_handle structure which holds the handle to be closed.
 * @return KAPI
 */
KAPI void filesystem_close(file_handle* handle);

/**
 * @brief Attempts to read the size of the file to which handle is attached.
 *
 * @param handle The file handle.
 * @param out_size A pointer to hold the file size.
 * @return KAPI
 */
KAPI b8 filesystem_size(file_handle* handle, u64* out_size);

/**
 * @brief  Reads up to a newline or EOF.
 * @param max_length The maximum length to be read from the line.
 * @param line_buf A pointer to a character array populated by this method. Must already be allocated.
 * @param out_line_length A pointer to hold the line length read from the file.
 * @returns True if successful; otherwise false.
 */
KAPI b8 filesystem_read_line(file_handle* handle, u64 max_length, char** line_buf, u64* out_line_length);

/**
 * @brief Write text to the provided file, appending a '\n' afterward.
 *
 * @param handle A Pointer to a file_handle structure.
 * @param text The text to be written.
 * @return True if successful; otherwise false.
 */
KAPI b8 filesystem_write_line(file_handle* handle, const char* text);

/**
 * Reads up to data_size bytes of data into out_bytes_read.
 * Allocates *out_data, which must be freed by the caller.
 * @param handle A pointer to a file_handle structure.
 * @param data_size The number of bytes to read.
 * @param out_data A pointer to a block of memory to be populated by this method.
 * @param out_bytes_read A pointer to a number which will be populated with the number of bytes actually read from the file.
 * @returns True if successful; otherwise false.
 */
KAPI b8 filesystem_read(file_handle* handle, u64 data_size, void* out_data, u64* out_bytes_read);

/**
 * Reads all bytes of data into out_bytes.
 * @param handle A pointer to a file_handle structure.
 * @param out_bytes A byte array which will be populated by this method.
 * @param out_bytes_read A pointer to a number which will be populated with the number of bytes actually read from the file.
 * @returns True if successful; otherwise false.
 */
KAPI b8 filesystem_read_all_bytes(file_handle* handle, u8* out_bytes, u64* out_bytes_read);

/**
 * Reads all characters of data into out_text.
 * @param handle A pointer to a file_handle structure.
 * @param out_text A character array which will be populated by this method.
 * @param out_bytes_read A pointer to a number which will be populated with the number of bytes actually read from the file.
 * @returns True if successful; otherwise false.
 */
KAPI b8 filesystem_read_all_text(file_handle* handle, char* out_text, u64* out_bytes_read);

/**
 * Writes provided data to the file.
 * @param handle A pointer to a file_handle structure.
 * @param data_size The size of the data in bytes.
 * @param data The data to be written.
 * @param out_bytes_written A pointer to a number which will be populated with the number of bytes actually written to the file.
 * @returns True if successful; otherwise false.
 */
KAPI b8 filesystem_write(file_handle* handle, u64 data_size, const void* data, u64* out_bytes_written);

/**
 * @brief Opens and reads all text content of the file at the provided path.
 * No file handle required. File is closed automatically.
 * NOTE: This function also handles size disparity between text read in and files that contain CRLF.
 *
 * @param filepath The path to the file to read.
 * @returns A string containing the file contents if successful; otherwise 0/null.
 */
KAPI const char* filesystem_read_entire_text_file(const char* filepath);