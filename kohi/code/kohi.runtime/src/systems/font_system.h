#pragma once

#include "math/math_types.h"
#include "renderer/renderer_types.h"
#include "resources/resource_types.h"

typedef struct system_font_config {
    /** @brief The name of the font. */
    const char* name;
    /** @brief The default size of the font. */
    u16 default_size;
    /** @brief The name of the resource containing the font data. */
    const char* resource_name;
} system_font_config;

typedef struct bitmap_font_config {
    /** @brief The name of the font. */
    const char* name;
    u16 size;
    /** @brief The name of the resource containing the font data. */
    const char* resource_name;
} bitmap_font_config;

typedef struct font_system_config {
     /** @brief The default system font config. */
    system_font_config default_system_font;
    /** @brief The default bitmap font config. */
    bitmap_font_config default_bitmap_font;
b8 auto_release;
} font_system_config;

b8 font_system_deserialize_config(const char* config_str, font_system_config* out_config);

b8 font_system_initialize(u64* memory_requirement, void* memory, font_system_config* config);
void font_system_shutdown(void* memory);
/**
 * @brief Loads a system font from the following config.
 *
 * @param config A pointer to the config to use for loading.
 * @return True on success; otherwise false.
 */
KAPI b8 font_system_system_font_load(system_font_config* config);
/**
 * @brief Loads a bitmap font from the following config.
 *
 * @param config A pointer to the config to use for loading.
 * @return True on success; otherwise false.
 */
KAPI b8 font_system_bitmap_font_load(bitmap_font_config* config);

/**
 * @brief Attempts to acquire a font of the given name and assign it to the given ui_text.
 *
 * @param font_name The name of the font to acquire. Must be an already loaded font.
 * @param font_size The font size. Ignored for bitmap fonts.
 * @param type The type of the font to acquire.
 * @return A pointer to font data if successful; otherwise 0/null.
 */
KAPI font_data* font_system_acquire(const char* font_name, u16 font_size, font_type type);

/**
 * @brief Releases references to the font held by the provided ui_text.
 *
 * @param font_name The name of the font to acquire. Must be an already loaded font.
 * @return True on success; otherwise false.
 */
KAPI b8 font_system_release(const char* font_name);
//校验图集
KAPI b8 font_system_verify_atlas(font_data* font, const char* text);

/**
 * @brief Measures the given string to find out how large it is at the widest/tallest point.
 *
 * @param font A pointer to the font to use for measuring.
 * @param text The text to be measured.
 */
KAPI vec2 font_system_measure_string(font_data* font, const char* text);
