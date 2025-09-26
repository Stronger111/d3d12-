

#include "assets/kasset_types.h"
#include "defines.h"
#include "kasset_kresource_utils.h"
#include "kresources/kresource_types.h"
#include "logger.h"
#include "memory/kmemory.h"
#include "serializers/kasset_material_serializer.h"
#include "strings/kname.h"
#include "systems/asset_system.h"
#include "systems/kresource_system.h"

// The number of channels per PBR material.
// i.e. albedo, normal, metallic/roughness/AO combined
//#define PBR_MATERIAL_CHANNEL_COUNT 3

typedef struct material_resource_handler_info {
    kresource_material* typed_resource;
    kresource_handler* handler;
    kresource_material_request_info* request_info;
    kasset_material* asset;
} material_resource_handler_info;

static void material_kasset_on_result(asset_request_result result, const struct kasset* asset, void* listener_inst);
static void asset_to_resource(const kasset_material* asset, kresource_material* out_material);

kresource* kresource_handler_material_allocate(void) {
    return (kresource*)KALLOC_TYPE(kresource_material, MEMORY_TAG_RESOURCE);
}

b8 kresource_handler_material_request(kresource_handler* self, kresource* resource, const struct kresource_request_info* info) {
    if (!self || !resource) {
        KERROR("kresource_handler_material_request requires valid pointers to self and resource.");
        return false;
    }

    kresource_material* typed_resource = (kresource_material*)resource;
    kresource_material_request_info* typed_request = (kresource_material_request_info*)info;
    typed_resource->base.state = KRESOURCE_STATE_UNINITIALIZED;

    if (info->assets.base.length != 1) {
        if (info->assets.base.length == 0 && typed_request->material_source_text) {
            //Deserialize material asset from provided source.
            kasset material_from_source = { 0 };
            if (!kasset_material_deserialize(typed_request->material_source_text, &material_from_source)) {
                KERROR("Failed to deserialize material from direct source upon resource request.");
                return false;
            }
            asset_to_resource((kasset_material*)&material_from_source, typed_resource);
            return true;
        }
        else {
            KERROR("kresource_handler_material_request requires exactly one asset OR zero assets and material source text.");
            return false;
        }
    }

    // NOTE: dynamically allocating this so lifetime isn't a concern.
    material_resource_handler_info* listener_inst = kallocate(sizeof(material_resource_handler_info), MEMORY_TAG_RESOURCE);
    // Take a copy of the typed request info.
    listener_inst->request_info = kallocate(sizeof(kresource_material_request_info), MEMORY_TAG_RESOURCE);
    kcopy_memory(listener_inst->request_info, typed_request, sizeof(kresource_material_request_info));
    listener_inst->typed_resource = typed_resource;
    listener_inst->handler = self;
    listener_inst->asset = 0;

    typed_resource->base.state = KRESOURCE_STATE_INITIALIZED;

    typed_resource->base.state = KRESOURCE_STATE_LOADING;

    kresource_asset_info* asset_info = &info->assets.data[0];
    asset_system_request(
        self->asset_system,
        asset_info->type,
        asset_info->package_name,
        asset_info->asset_name,
        true,
        listener_inst,
        material_kasset_on_result,
        0,
        0);

    return true;
}

void kresource_handler_material_release(kresource_handler* self, kresource* resource) {
    if (resource) {
        kresource_material* typed_resource = (kresource_material*)resource;

        if (typed_resource->custom_sampler_count && typed_resource->custom_samplers) {
            KFREE_TYPE_CARRAY(typed_resource->custom_samplers, kasset_material_sampler, typed_resource->custom_sampler_count);
        }

        KFREE_TYPE(typed_resource, kresource_material, MEMORY_TAG_RESOURCE);
    }
}

// static b8 process_asset_material_map(kname material_name, kasset_material_map* map, kresource_texture_map* target_map) {
//     target_map->repeat_u = map->repeat_u;
//     target_map->repeat_v = map->repeat_v;
//     target_map->repeat_w = map->repeat_w;
//     target_map->filter_minify = map->filter_min;
//     target_map->filter_magnify = map->filter_mag;
//     target_map->internal_id = 0;
//     target_map->generation = INVALID_ID;
//     target_map->mip_levels = 0; // TODO: Do we need this?
//     if (!renderer_kresource_texture_map_resources_acquire(engine_systems_get()->renderer_system, target_map)) {
//         KERROR("Failed to acquire texture map resources for material '%s', for the '%s' map.", material_name, map->name);
//         return false;
//     }

//     target_map->texture = texture_system_request(
//         map->image_asset_name,
//         map->image_asset_package_name,
//         0,
//         0);

//     return true;
// }

// typedef enum mra_state {
//     MRA_STATE_UNINITIALIZED,
//     MRA_STATE_REQUESTED,
//     MRA_STATE_LOADED
// } mra_state;

// typedef enum mra_indices {
//     MRA_INDEX_METALLIC = 0,
//     MRA_INDEX_ROUGHNESS = 1,
//     MRA_INDEX_AO = 2
// } mra_indices;

// typedef struct material_mra_data {
//     mra_state state;
//     kasset_material_model channel;
//     kname image_asset_name;
//     kname image_asset_package_name;
//     kasset_image* asset;
// } material_mra_data;

// typedef struct material_mra_listener {
//     kresource_texture_map* metallic_roughness_ao_map;
//     kasset_material_map metallic_roughness_ao_map_config;
//     material_mra_data map_assets[3];
//     kresource_material* typed_resource;
// } material_mra_listener;

// static void material_on_metallic_roughness_ao_image_asset_loaded(asset_request_result result, const struct kasset* asset, void* listener_inst) {
//     if (result == ASSET_REQUEST_RESULT_SUCCESS) {
//         material_mra_listener* listener = (material_mra_listener*)listener_inst;

//         // Test for which asset loaded.
//         for (u32 i = 0; i < 3; ++i) {
//             material_mra_data* m = &listener->map_assets[i];

//             if (m->image_asset_name == asset->name) {
//                 m->state = MRA_STATE_LOADED;
//                 m->asset = (kasset_image*)asset;
//                 break;
//             }
//         }

//         // Boot if we are waiting on an asset to finish loading.
//         for (u32 i = 0; i < 3; ++i) {
//             material_mra_data* m = &listener->map_assets[i];
//             if (m->state == MRA_STATE_REQUESTED) {
//                 KTRACE("Still waiting on asset '%s'...", kname_string_get(m->image_asset_name));
//                 return;
//             }
//         }

//         // This means everything that was request to load has loaded, and combination of asset channel data may begin.
//         u32 width = U32_MAX, height = U32_MAX;
//         u8* pixels = 0;
//         u32 pixel_array_size = 0;
//         for (u32 i = 0; i < 3; ++i) {
//             material_mra_data* m = &listener->map_assets[i];
//             if (m->state == MRA_STATE_LOADED) {
//                 if (width == U32_MAX || height == U32_MAX) {
//                     width = m->asset->width;
//                     height = m->asset->height;
//                     pixel_array_size = sizeof(u8) * width * height * 4;
//                     pixels = kallocate(pixel_array_size, MEMORY_TAG_RESOURCE);
//                 }
//                 else if (width != m->asset->width || height != m->asset->height) {
//                     KWARN("All assets for material metallic, roughness and AO maps must be the same resolution. Default data will be used instead.");
//                     // Use default data instead by releasing the asset and resetting the state.
//                     asset_system_release(engine_systems_get()->asset_state, m->image_asset_name, m->image_asset_package_name);
//                     m->asset = 0;
//                     m->state = MRA_STATE_UNINITIALIZED;
//                 }
//             }
//             else if (m->state == MRA_STATE_UNINITIALIZED) {
//                 // TODO: Use default data instead.
//             }
//         }

//         if (!pixels) {
//             KERROR("Pixel array not created during asset load for material. This likely means other errors have occurred. Check logs.");
//             return;
//         }

//         for (u32 i = 0; i < 3; ++i) {
//             material_mra_data* m = &listener->map_assets[i];
//             if (m->state == MRA_STATE_LOADED) {
//                 u8 offset = 0;
//                 switch (m->channel) {
//                 default:
//                 case KASSET_MATERIAL_MAP_CHANNEL_METALLIC:
//                     offset = 0;
//                     break;
//                 case KASSET_MATERIAL_MAP_CHANNEL_ROUGHNESS:
//                     offset = 1;
//                     break;

//                 case KASSET_MATERIAL_MAP_CHANNEL_AO:
//                     offset = 2;
//                     break;
//                 }
//                 for (u64 row = 0; row < height; ++row) {
//                     for (u64 col = 0; col < width; ++col) {
//                         u64 index = (row * width) + col;
//                         u64 index_bpp = index * 4;
//                         pixels[index_bpp + offset] = m->asset->pixels[index_bpp + 0]; // Pull from the red channel
//                     }
//                 }

//             }
//             else if (m->state == MRA_STATE_UNINITIALIZED) {
//                 // Use default data instead.
//                 u32 offset = 0;
//                 u8 value = 0;
//                 switch (m->channel) {
//                 default:
//                 case KASSET_MATERIAL_MAP_CHANNEL_METALLIC:
//                     offset = 0;
//                     value = 0; // Default for metallic is black.
//                     break;
//                 case KASSET_MATERIAL_MAP_CHANNEL_ROUGHNESS:
//                     offset = 1;
//                     value = 128; // Default for roughness is medium grey
//                     break;

//                 case KASSET_MATERIAL_MAP_CHANNEL_AO:
//                     offset = 2;
//                     value = 255; // Default for AO is white.
//                     break;
//                 }

//                 for (u64 row = 0; row < height; ++row) {
//                     for (u64 col = 0; col < width; ++col) {
//                         u64 index = (row * width) + col;
//                         u64 index_bpp = index * 4;
//                         pixels[index_bpp + offset] = value;
//                     }
//                 }
//             }
//         }

//         if (!listener->metallic_roughness_ao_map->texture) {

//             kresource_texture_map* target_map = listener->metallic_roughness_ao_map;
//             target_map->repeat_u = listener->metallic_roughness_ao_map_config.repeat_u;
//             target_map->repeat_v = listener->metallic_roughness_ao_map_config.repeat_v;
//             target_map->repeat_w = listener->metallic_roughness_ao_map_config.repeat_w;
//             target_map->filter_minify = listener->metallic_roughness_ao_map_config.filter_min;
//             target_map->filter_magnify = listener->metallic_roughness_ao_map_config.filter_mag;
//             target_map->internal_id = 0;
//             target_map->generation = INVALID_ID;
//             target_map->mip_levels = 0; // TODO: Do we need this?

//             // Setup texture map resources.
//             if (!renderer_kresource_texture_map_resources_acquire(engine_systems_get()->renderer_system, target_map)) {
//                 KERROR("Failed to acquire texture map resources for material '%s', for the '%s' map.", kname_string_get(listener->typed_resource->base.name), listener->metallic_roughness_ao_map_config.name);
//                 return;
//             }

//             const char* map_name = string_format("%s_metallic_roughness_ao_generated", listener->typed_resource->base.name);
//             listener->typed_resource->metallic_roughness_ao_map.texture = texture_system_request_writeable(
//                 kname_create(map_name),
//                 width, height, KRESOURCE_TEXTURE_FORMAT_RGBA8, false, false);
//             string_free(map_name);

//             if (!texture_system_write_data((kresource_texture*)listener->metallic_roughness_ao_map->texture, 0, pixel_array_size, pixels)) {
//                 KERROR("Failed to upload combined texture data for material resource '%s'", kname_string_get(listener->typed_resource->base.name));
//                 return;
//             }

//             KTRACE("Successfully uploaded combined texture data for material resource '%s'", kname_string_get(listener->typed_resource->base.name));
//         }

//     }
//     else {
//         KERROR("Asset failed to load. See logs for details.");
//     }
// }

static void material_kasset_on_result(asset_request_result result, const struct kasset* asset, void* listener_inst) {
    material_resource_handler_info* listener = (material_resource_handler_info*)listener_inst;
    if (result == ASSET_REQUEST_RESULT_SUCCESS) {
        // Save off the asset pointer to the array.
        listener->asset = (kasset_material*)asset;
        asset_to_resource(listener->asset, listener->typed_resource);
    }
    else {
        KERROR("Failed to load a required asset for material resource '%s'. Resource may not appear correctly when rendered.", kname_string_get(listener->typed_resource->base.name));
    }
    // Destroy the request.
    array_kresource_asset_info_destroy(&listener->request_info->base.assets);
    kfree(listener->request_info, sizeof(kresource_material_request_info), MEMORY_TAG_RESOURCE);
    // Free the listener itself.
    kfree(listener, sizeof(material_resource_handler_info), MEMORY_TAG_RESOURCE);
}

static void asset_to_resource(const kasset_material* asset, kresource_material* out_material) {
    // Take a copy of all of the asset properties.

    out_material->type = kasset_material_type_to_kresource(asset->type);
    out_material->model = kasset_material_model_to_kresource(asset->model);

    out_material->has_transparency = asset->has_transparency;
    out_material->double_sided = asset->double_sided;
    out_material->recieves_shadow = asset->recieves_shadow;
    out_material->casts_shadow = asset->casts_shadow;
    out_material->use_vertex_colour_as_base_colour = asset->use_vertex_colour_as_base_colour;

    out_material->custom_shader_name = asset->custom_shader_name;

    out_material->base_colour = asset->base_colour;
    out_material->base_colour_map = kasset_material_texture_to_kresource(asset->base_colour_map);

    out_material->normal_enabled = asset->normal_enabled;
    out_material->normal = asset->normal;
    out_material->normal_map = kasset_material_texture_to_kresource(asset->normal_map);

    out_material->metallic = asset->metallic;
    out_material->metallic_map = kasset_material_texture_to_kresource(asset->metallic_map);
    out_material->metallic_map_source_channel = kasset_material_tex_map_channel_to_kresource(asset->metallic_map_source_channel);

    out_material->roughness = asset->roughness;
    out_material->roughness_map = kasset_material_texture_to_kresource(asset->roughness_map);
    out_material->roughness_map_source_channel = kasset_material_tex_map_channel_to_kresource(asset->roughness_map_source_channel);

    out_material->ambient_occlusion_enabled = asset->ambient_occlusion_enabled;
    out_material->ambient_occlusion = asset->ambient_occlusion;
    out_material->ambient_occlusion_map = kasset_material_texture_to_kresource(asset->ambient_occlusion_map);
    out_material->ambient_occlusion_map_source_channel = kasset_material_tex_map_channel_to_kresource(asset->ambient_occlusion_map_source_channel);

    out_material->mra = asset->mra;
    out_material->mra_map = kasset_material_texture_to_kresource(asset->mra_map);
    out_material->use_mra = asset->use_mra;

    out_material->emissive_enabled = asset->emissive_enabled;
    out_material->emissive = asset->emissive;
    out_material->emissive_map = kasset_material_texture_to_kresource(asset->emissive_map);

    out_material->custom_sampler_count = asset->custom_sampler_count;
    KALLOC_TYPE_CARRAY(kasset_material_sampler, out_material->custom_sampler_count);
    KCOPY_TYPE_CARRAY(
        out_material->custom_samplers,
        asset->custom_samplers,
        kasset_material_sampler,
        out_material->custom_sampler_count);

    out_material->base.state = KRESOURCE_STATE_LOADED;
}