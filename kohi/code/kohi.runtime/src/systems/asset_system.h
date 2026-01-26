/**
 * @file asset_system.h
 * @author Travis Vroman (travis@kohiengine.com)
 * @brief This files contains the implementation of the asset system, which
 * is responsible for managing the lifecycle of assets.
 *
 * @details
 * @version 1.0
 * @date 2024-07-28
 *
 * @copyright Kohi Game Engine is Copyright (c) Travis Vroman 2021-2024
 *
 */

#pragma once

#include <assets/kasset_types.h>
#include <strings/kname.h>

typedef struct asset_system_config {
    // The maximum number of assets which may be loaded at once.
    u32 max_asset_count;

    kname application_package_name;
    const char* application_package_name_str;
}asset_system_config;

struct asset_system_state;

/**
 * @brief Deserializes configuration for the asset system from the provided string.
 *
 * @param config_str The string to deserialize.
 * @param out_config A pointer to hold the deserialized config.
 * @return True on success; otherwise false.
 */
KAPI b8 asset_system_deserialize_config(const char* config_str, asset_system_config* out_config);

/**
 * @brief Initializes the asset system. Call twice; once to get the memory requirement (pass 0 to state and config) and a second
 * time passing along the state and config once allocated.
 *
 * @param memory_requirement A pointer to hold the numeric amount of bytes needed for the state. Required.
 * @param state A pointer to the state. Pass 0 when getting memory requirement, otherwise pass the block of allocated memory.
 * @param config A constant pointer to the configuration of the system. Ignored when getting memory requirement.
 * @return True on success; otherwise false.
 */
KAPI b8 asset_system_initialize(u64* memory_requirement, struct asset_system_state* state, const asset_system_config* config);

/**
 * @brief Shuts the system down.
 *
 * @param state A pointer to the state. Required.
 */
KAPI void asset_system_shutdown(struct asset_system_state* state);

// ////////////////////////////////////
// BINARY ASSETS
// ////////////////////////////////////
typedef void (*PFN_kasset_binary_loaded_callback)(void* listener, kasset_binary* asset);
//async load from game package.
KAPI kasset_binary* asset_system_request_binary(struct asset_system_state* state, const char* name, void* listener, PFN_kasset_binary_loaded_callback callback);
//sync load from game package.
KAPI kasset_binary* asset_system_request_binary_sync(struct asset_system_state* state, const char* name);
//async load from specific package.
KAPI kasset_binary* asset_system_request_binary_from_package(struct asset_system_state* state, const char* package_name, const char* name, void* listener, PFN_kasset_binary_loaded_callback callback);
//sync load from specific package.
KAPI kasset_binary* asset_system_request_binary_from_package_sync(struct asset_system_state* state, const char* package_name, const char* name);

KAPI void asset_system_release_binary(struct asset_system_state* state, kasset_binary* asset);

// ////////////////////////////////////
// IMAGE ASSETS
// ////////////////////////////////////

typedef void (*PFN_kasset_image_loaded_callback)(void* listener, kasset_image* asset);
// async load from game package.
KAPI kasset_image* asset_system_request_image(struct asset_system_state* state, const char* name, b8 flip_y, void* listener, PFN_kasset_image_loaded_callback callback);
// sync load from game package.
KAPI kasset_image* asset_system_request_image_sync(struct asset_system_state* state, const char* name, b8 flip_y);
// async load from specific package.
KAPI kasset_image* asset_system_request_image_from_package(struct asset_system_state* state, const char* package_name, const char* name, b8 flip_y, void* listener, PFN_kasset_image_loaded_callback callback);
// sync load from specific package.
KAPI kasset_image* asset_system_request_image_from_package_sync(struct asset_system_state* state, const char* package_name, const char* name, b8 flip_y);

KAPI void asset_system_release_image(struct asset_system_state* state, kasset_image* asset);

// /**
//  * @brief Requests an asset by type, name and package name. This operation is asynchronus, and will provide its result via a
//  * callback (if provided) at a later time. Internally, a reference count for each asset is maintained each time the asset
//  * is requested. If the asset's first request had auto-release set to true, it will be released automatically when this
//  * count reaches 0.
//  *
//  * @param A pointer to the asset system state. Required.
//  * @param info The information about the asset request.
//  */
// KAPI void asset_system_request(struct asset_system_state* state, asset_request_info info);

// /**
//  * @brief Releases an asset via the fully-qualified name.
//  *
//  * @param A pointer to the asset system state. Required.
//  * @param asset_name The name of the asset to be released.
//  * @param package_name The name of the package containing the asset.
//  */
// KAPI void asset_system_release(struct asset_system_state* state, kname asset_name, kname package_name);

// /**
//  * @brief A callback function to be made from an asset handler when an asset is fully loaded and ready to go.
//  *
//  * @param A pointer to the asset system state. Required.
//  * @param result The result of the load operation.
//  * @param A pointer to the asset used in the operation.
//  */
// KAPI void asset_system_on_handler_result(struct asset_system_state* state, asset_request_result  result, kasset* asset, void* listener_instance, PFN_kasset_on_result callback);

// /**
//  * @brief Indicates if the provided asset type is a binary asset.
//  *
//  * @param type The asset type.
//  * @return True if binary; otherwise treated as text.
//  */
// KAPI b8 asset_type_is_binary(kasset_type type);

// void asset_system_register_hot_reload_callback(struct asset_system_state* state, void* listener, PFN_asset_system_hot_reload_callback callback);