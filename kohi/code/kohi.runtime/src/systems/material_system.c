#include "material_system.h"

#include "assets/kasset_types.h"
#include "containers/darray.h"
#include "core/console.h"
#include "core/engine.h"
#include "kdebug/kassert.h"
#include "defines.h"
#include "identifiers/khandle.h"
#include "kresources/kresource_types.h"
#include "kresources/kresource_utils.h"
#include "logger.h"
#include "math/kmath.h"
#include "memory/kmemory.h"
#include "renderer/renderer_frontend.h"
#include "renderer/renderer_types.h"
#include "resources/resource_types.h"
#include "serializers/kasset_material_serializer.h"
#include "strings/kname.h"
#include "systems/kresource_system.h"
#include "systems/shader_system.h"
#include "systems/texture_system.h"

#define MATERIAL_SHADER_NAME_STANDARD "Shader.MaterialStandard"
#define MATERIAL_SHADER_NAME_WATER "Shader.MaterialWater"
#define MATERIAL_SHADER_NAME_BLENDED "Shader.MaterialBlended"

// Textures
const u32 MAT_STANDARD_IDX_BASE_COLOUR = 0;
const u32 MAT_STANDARD_IDX_NORMAL = 1;
const u32 MAT_STANDARD_IDX_METALLIC = 2;
const u32 MAT_STANDARD_IDX_ROUGHNESS = 3;
const u32 MAT_STANDARD_IDX_AO = 4;
const u32 MAT_STANDARD_IDX_MRA = 5;
const u32 MAT_STANDARD_IDX_EMISSIVE = 6;
const u32 MAT_STANDARD_IDX_SHADOW_MAP = 7;
const u32 MAT_STANDARD_IDX_IRRADIANCE_MAP = 8;

#define SHADOW_CASCADE_COUNT 4

// TODO:
// - Water type material
// - Blended type material
// - Material models (unlit, PBR, Phong, etc.)
// - Shader interaction/binding/applying for material instances

// Represents the data for a single instance of a material.
// This can be thought of as "per-draw" data.
typedef struct material_instance_data {
    // Instance has recieved and update that needs to be written to the renderer.
    b8 is_dirty;
    // A handle to the material to which this instance references.
    khandle material;
    // A unique id used for handle validation.
    u64 unique_id;
    // Shader draw id for per-draw uniforms.
    u32 per_draw_id;

    // Multiplied by albedo/diffuse texture. Overrides the value set in the base material.
    vec4 base_colour;

    // Overrides the flags set in the base material.
    material_flags flags;

    // Added to UV coords of vertex data.
    vec3 uv_offset;
    // Multiplied against uv coords of vertex data.
    vec3 uv_scale;
} material_instance_data;

// Represents a base material.
// This can be thought of as "per-group" data.
typedef struct material_data {
    kname name;
    /** @brief The material type. Ultimately determines what shader the material is rendered with. */
    material_type type;
    /** @brief The material lighting model. */
    material_model model;
    // A unique id used for handle validation.
    u64 unique_id;

    vec4 base_colour;
    kresource_texture* base_colour_texture;

    kresource_texture* normal_texture;

    f32 metallic;
    kresource_texture* metallic_texture;
    texture_channel metallic_texture_channel;

    f32 roughness;
    kresource_texture* roughness_texture;
    texture_channel roughness_texture_channel;

    f32 ao;
    kresource_texture* ao_texture;
    texture_channel ao_texture_channel;

    vec4 emissive;
    kresource_texture* emissive_texture;
    f32 emissive_texture_intensity;

    kresource_texture* refraction_texture;
    f32 refraction_scale;

    vec3 mra;
    /**
    * @brief This is a combined texture holding metallic/roughness/ambient occlusion all in one texture.
    * This is a more efficient replacement for using those textures individually. Metallic is sampled
    * from the Red channel, roughness from the Green channel, and ambient occlusion from the Blue channel.
    * Alpha is ignored.
    */
    kresource_texture* mra_texture;
    // Base set of flags for the material. Copied to the material instance when created.
    material_flags flags;
    // Added to UV coords of vertex data. Overridden by instance data.
    vec3 uv_offset;
    // Multiplied against uv coords of vertex data. Overridden by instance data.
    vec3 uv_scale;
    // Shader group id for per-group uniforms.
    u32 group_id;
    // The frame number where the per-group uniforms were last synced.
    u64 renderer_frame_number;
}material_data;

typedef enum material_standard_flag_bits {
    MATERIAL_STANDARD_FLAG_USE_BASE_COLOUR_TEX = 0x0001,
    MATERIAL_STANDARD_FLAG_USE_NORMAL_TEX = 0x0002,
    MATERIAL_STANDARD_FLAG_USE_METALLIC_TEX = 0x0004,
    MATERIAL_STANDARD_FLAG_USE_ROUGHNESS_TEX = 0x0008,
    MATERIAL_STANDARD_FLAG_USE_AO_TEX = 0x0010,
    MATERIAL_STANDARD_FLAG_USE_MRA_TEX = 0x0020,
    MATERIAL_STANDARD_FLAG_USE_EMISSIVE_TEX = 0x0040
} material_standard_flag_bits;

typedef u32 material_standard_flags;

typedef struct material_standard_shader_locations {
    u16 projection;
    u16 views;
    u16 cascade_splits;
    u16 view_positions;
    u16 properties;
    u16 ibl_cube_textures;
    u16 material_textures;
    u16 shadow_textures;
    u16 light_space_0;
    u16 light_space_1;
    u16 light_space_2;
    u16 light_space_3;
    u16 model;
    u16 render_mode;
    u16 use_pcf;
    u16 bias;
    u16 clipping_plane;
    u16 view_index;
    u16 ibl_index;
    u16 dir_light;
    u16 p_lights;
    u16 num_p_lights;
    u16 base_colour;
    u16 normal;
    u16 metallic;
    u16 metallic_source_channel;
    u16 roughness;
    u16 roughness_source_channel;
    u16 ao;
    u16 ao_source_channel;
    u16 emissive;
    u16 mra;
    u16 flags;
    // Texture use flags
    u16 tex_flags;
    u16 uv_offset;
    u16 uv_scale;
} material_standard_shader_locations;

typedef struct material_standard_frame_data {
    mat4 projection;
    mat4 view;
    vec3 view_position;
    mat4 inv_view;
    vec3 inv_view_position;
    u32 render_mode;
    vec4 cascade_splits[SHADOW_CASCADE_COUNT];
    // Light space for shadow mapping. Per cascade
    mat4 directional_light_spaces[SHADOW_CASCADE_COUNT];
    // HACK: Read this in from somewhere (or have global setter?);
    f32 bias;
    vec4 clipping_plane;
}material_standard_frame_data;

typedef struct material_system_state {
    material_system_config config;

    // darray of materials, indexed by material khandle resource index.
    material_data* materials;
    // darray of material instances, indexed first by material khandle index, then by instance khandle index.
    material_instance_data** instances;

    // A default material for each type of material.
    khandle default_standard_material;
    khandle default_water_material;
    khandle default_blended_material;
    material_standard_shader_locations standard_material_locations;
    material_standard_frame_data standard_frame_data;

    // Cached handles for various material types' shaders.
    khandle material_standard_shader;
    khandle material_water_shader;
    khandle material_blended_shader;

    // Keep a pointer to the renderer state for quick access.
    struct renderer_system_state* renderer;
    struct texture_system_state* texture_system;
    struct kresource_system_state* resource_system;

} material_system_state;

typedef struct material_request_listener {
    khandle material_handle;
    khandle* instance_handle;
    material_system_state* state;
}material_request_listener;

static b8 create_default_standard_material(material_system_state* state);
static b8 create_default_water_material(material_system_state* state);
static b8 create_default_blended_material(material_system_state* state);
static void on_material_system_dump(console_command_context context);
static khandle get_shader_for_material_type(const material_system_state* state, material_type type);
static khandle material_handle_create(material_system_state* state, kname name);
static b8 material_instance_handle_create(material_system_state* state, khandle material_handle);
static b8 material_create(material_system_state* state, khandle material_handle, const kresource_material* typed_resource);
static void material_destroy(material_system_state* state, khandle* material_handle);
static b8 material_instance_create(material_system_state* state, khandle base_material, khandle* out_instance_handle);
static void material_instance_destroy(material_system_state* state, khandle base_material, khandle* instance_handle);
static void material_resource_loaded(kresource* resource, void* listener);
static material_instance default_material_instance_get(material_system_state* state, khandle base_material, const char* name_str);
static material_instance_data* get_instance_data(material_system_state* state, material_instance instance);
static void default_standard_material_locations_get(material_system_state* state);
//static b8 assign_map(material_system_state* state, kresource_texture_map* map, const material_map* config, kname material_name, const kresource_texture* default_tex);
// NEW
// static khandle material_create(material_system_state* state, const kresource_material* typed_resource) {
//     u32 resource_index = INVALID_ID;

//     // Attempt to find a free "slot", or create a new entry if there isn't one.
//     u32 material_count = darray_length(state->materials);
//     for (u32 i = 0; i < material_count; ++i) {
//         if (state->materials[i].unique_id == INVALID_ID_U64) {
//             // free slot. An array should already exists for instances here.
//             resource_index = i;
//             break;
//         }
//     }
//     if (resource_index == INVALID_ID) {
//         resource_index = material_count;
//         darray_push(state->materials, (material_data) { 0 });
//         // This also means a new entry needs to be created at this index for instances.
//         material_instance_data* new_inst_array = darray_create(material_instance_data);
//         darray_push(state->instances, new_inst_array);
//     }

//     material_data* material = &state->materials[resource_index];

//     // Setup a handle first.
//     khandle handle = khandle_create(resource_index);
//     material->unique_id = handle.unique_id.uniqueid;

//     // Base colour map or value
//     if (typed_resource->base_colour_map.resource_name) {
//         material->base_colour_texture = texture_system_request(typed_resource->base_colour_map.resource_name, typed_resource->base_colour_map.package_name, 0, 0);
//     }
//     else {
//         material->base_colour = typed_resource->base_colour;
//     }

//     // Normal map
//     if (typed_resource->normal_map.resource_name) {
//         material->normal_texture = texture_system_request(typed_resource->normal_map.resource_name, typed_resource->normal_map.package_name, 0, 0);
//     }
//     material->flags |= typed_resource->normal_enabled ? MATERIAL_FLAG_NORMAL_ENABLED_BIT : 0;

//     // Metallic map or value
//     if (typed_resource->metallic_map.resource_name) {
//         material->metallic_texture = texture_system_request(typed_resource->metallic_map.resource_name, typed_resource->metallic_map.package_name, 0, 0);
//         material->metallic_texture_channel = kresource_texture_map_channel_to_texture_channel(typed_resource->metallic_map.channel);
//     }
//     else {
//         material->metallic = typed_resource->metallic;
//     }
//     // Roughness map or value
//     if (typed_resource->roughness_map.resource_name) {
//         material->roughness_texture = texture_system_request(typed_resource->roughness_map.resource_name, typed_resource->roughness_map.package_name, 0, 0);
//         material->roughness_texture_channel = kresource_texture_map_channel_to_texture_channel(typed_resource->roughness_map.channel);
//     }
//     else {
//         material->roughness = typed_resource->roughness;
//     }
//     // Ambient occlusion map or value
//     if (typed_resource->ambient_occlusion_map.resource_name) {
//         material->ao_texture = texture_system_request(typed_resource->ambient_occlusion_map.resource_name, typed_resource->ambient_occlusion_map.package_name, 0, 0);
//         material->ao_texture_channel = kresource_texture_map_channel_to_texture_channel(typed_resource->ambient_occlusion_map.channel);
//     }
//     else {
//         material->ao = typed_resource->ambient_occlusion;
//     }
//     material->flags |= typed_resource->ambient_occlusion_enabled ? MATERIAL_FLAG_AO_ENABLED_BIT : 0;

//     // MRA (combined metallic/roughness/ao) map or value
//     if (typed_resource->mra_map.resource_name) {
//         material->mra_texture = texture_system_request(typed_resource->mra_map.resource_name, typed_resource->mra_map.package_name, 0, 0);
//     }
//     else {
//         material->mra = typed_resource->mra;
//     }
//     material->flags |= typed_resource->use_mra ? MATERIAL_FLAG_MRA_ENABLED_BIT : 0;

//     // Emissive map or value
//     if (typed_resource->emissive_map.resource_name) {
//         material->emissive_texture = texture_system_request(typed_resource->emissive_map.resource_name, typed_resource->emissive_map.package_name, 0, 0);
//     }
//     else {
//         material->emissive = typed_resource->emissive;
//     }
//     material->flags |= typed_resource->emissive_enabled ? MATERIAL_FLAG_EMISSIVE_ENABLED_BIT : 0;

//     // Set remaining flags
//     material->flags |= typed_resource->has_transparency ? MATERIAL_FLAG_HAS_TRANSPARENCY : 0;
//     material->flags |= typed_resource->double_sided ? MATERIAL_FLAG_DOUBLE_SIDED_BIT : 0;
//     material->flags |= typed_resource->recieves_shadow ? MATERIAL_FLAG_RECIEVES_SHADOW_BIT : 0;
//     material->flags |= typed_resource->casts_shadow ? MATERIAL_FLAG_CASTS_SHADOW_BIT : 0;
//     material->flags |= typed_resource->use_vertex_colour_as_base_colour ? MATERIAL_FLAG_USE_VERTEX_COLOUR_AS_BASE_COLOUR : 0;

//     // LEFTOFF: Setup shader resources, etc.
//     //
//     // Create a group for the material.

//     return handle;
// }

b8 material_system_initialize(u64* memory_requirement, material_system_state* state, const material_system_config* config) {
    material_system_config* typed_config = (material_system_config*)config;
    if (typed_config->max_material_count == 0) {
        KFATAL("material_system_initialize - config.max_material_count must be > 0.");
        return false;
    }

    // Block of memory will contain state structure, then block for array, then block for hashtable.
    *memory_requirement = sizeof(material_system_state);

    if (!state) {
        return true;
    }

    // Keep a pointer to the renderer system state for quick access.
    const engine_system_states* states = engine_systems_get();
    state->renderer = states->renderer_system;
    state->resource_system = states->kresource_state;
    state->texture_system = states->texture_system;

    state->config = *typed_config;

    state->materials = darray_create(material_data);
    // An array for each material will be created when a material is created.
    state->instances = darray_create(material_instance_data*);

    // Get default material shaders.
    state->material_standard_shader = shader_system_get(kname_create(MATERIAL_SHADER_NAME_STANDARD));
    default_standard_material_locations_get(state);
    // Setup per-frame data for the standard shader.
    state->standard_frame_data.projection = mat4_perspective(deg_to_rad(45.0f), 720.0f / 1280.0f, 0.01f, 1000.0f);
    state->standard_frame_data.inv_view = mat4_look_at(vec3_zero(), vec3_forward(), vec3_up());
    state->standard_frame_data.inv_view_position = vec3_zero();
    state->standard_frame_data.view = mat4_inverse(state->standard_frame_data.inv_view);
    state->standard_frame_data.view_position = vec3_zero();
    state->standard_frame_data.render_mode = 0;
    for (u32 i = 0; i < SHADOW_CASCADE_COUNT; ++i) {
        state->standard_frame_data.cascade_splits[i] = vec4_zero();
        state->standard_frame_data.directional_light_spaces[i] = mat4_identity();
    }
    state->standard_frame_data.use_pcf = 1;
    state->standard_frame_data.bias = 0.0005f;
    state->standard_frame_data.clipping_plane = vec4_zero();

    state->material_water_shader = shader_system_get(kname_create(MATERIAL_SHADER_NAME_WATER));
    state->material_blended_shader = shader_system_get(kname_create(MATERIAL_SHADER_NAME_BLENDED));

    // Load up some default materials
    if (!create_default_standard_material(state)) {
        KFATAL("Failed to create default standard material. Application cannot continue.");
    }

    if (!create_default_water_material(state)) {
        KFATAL("Failed to create default blended material. Application cannot continue.");
        return false;
    }

    if (!create_default_blended_material(state)) {
        KFATAL("Failed to create default blended material. Application cannot continue.");
        return false;
    }

    // Register a console command to dump list of materials/references.
    console_command_register("material_system_dump", 0, on_material_system_dump);
    return true;
}

void material_system_shutdown(struct material_system_state* state) {
    if (state) {

        // Destroy default materials.
        material_destroy(state, &state->default_standard_material);
        material_destroy(state, &state->default_water_material);
        material_destroy(state, &state->default_blended_material);

        // Release shaders for the default materials.
        shader_system_destroy(&state->material_standard_shader);
        shader_system_destroy(&state->material_water_shader);
        shader_system_destroy(&state->material_blended_shader);
    }
}

// static void material_resource_loaded(kresource* resource, void* listener) {
//     kresource_material* typed_resource = (kresource_material*)resource;
//     material_instance* instance = (material_instance*)listener;
//     // TODO: In this case, the texture map should probably actually be stored on the "probe" itself,
//     // this would reduce the number of samplers required. Scenes can either have a probe or not.
//     // If there is no probe, whatever is rendering the scene (i.e the forward rendergraph node) should have
//     // a default sampler in this case.
//     // Additionally, the "IBL cubemap" should be converted to a sampler array with a max number of samplers
//     // (say, 4 for example), and a local index should be passed indicating which one should be used per render.
//     // The IBL cubemap sampler array should be global.
//     // This will eliminate the need for local samplers which were just added, but should work best.
//     // LEFTOFF: If the resource is already loaded, then new local resources from the shader it is associated
//     // with must be obtained here before returning. If the resource is not yet loaded, then this
//     // should happen when the resource is finally loaded. This means the pointer to the local id
//     // will need to be passed along in the context of the request.
//     if (resource->state == KRESOURCE_STATE_LOADED) {
//         if (typed_resource->type == KRESOURCE_MATERIAL_MODEL_PBR) {
//             // FIXME: use kname instead
//             u32 pbr_shader_id = shader_system_get_id("Shader.PBRMaterial");
//             // NOTE:No maps for this shader type
//             if (!shader_system_shader_per_draw_acquire(pbr_shader_id, 1, 0, &instance->per_draw_id)) {
//                 KASSERT_MSG(false, "Failed to acquire renderer resources for default PBR material. Application cannot continue.");
//             }
//         }
//         else {
//             KASSERT_MSG(false, "Unsupported material type - add local shader acquisition logic.");
//         }
//     }
// }

b8 material_system_acquire(material_system_state* state, kname name, material_instance* out_instance) {
    KASSERT_MSG(out_instance, "out_instance is required.");

    u32 material_count = darray_length(state->materials);
    for (u32 i = 0;i < material_count;++i) {
        material_data* material = &state->materials[i];
        if (material->name == name) {
            // Material exists, create an instance and boot.
            out_instance->material = khandle_create_with_u64_identifier(i, material->unique_id);

            //Request instance and set handle.
            b8 instance_request = material_instance_create(state, out_instance->material, &out_instance->instance);
            if (!instance_request) {
                KERROR("Failed to create material instance during new material creation.");
            }
            return instance_request;
        }
    }

    // Material is not yet loaded, request it.
    // Setup a listener.
    material_request_listener* listener = KALLOC_TYPE(material_request_listener, MEMORY_TAG_MATERIAL_INSTANCE);
    listener->state = state;
    listener->material_handle = material_handle_create(state, name);
    listener->instance_handle = &out_instance->instance;
    // Request the resource.
    kresource_material_request_info request = { 0 };
    request.base.type = KRESOURCE_TYPE_MATERIAL;
    request.base.user_callback = material_resource_loaded;
    request.base.listener_inst = listener;
    kresource* r = kresource_system_request(state->resource_system, name, (kresource_request_info*)&request);
    return r != 0;
}

void material_system_release(material_system_state* state, material_instance* instance) {
    if (!state) {
        return;
    }

    // Getting the material instance data successfully performs all handle checks for
   // the material and instance. This means it's safe to destroy.
    if (get_instance_data(state, *instance)) {

        material_instance_destroy(state, instance->material, &instance->instance);
        // Invalidate the material handle in the instance pointer as well.
        khandle_invalidate(&instance->material);
    }
}

b8 material_system_prepare_frame(material_system_state* state) {
    if (!state) {
        return false;
    }

    // Standard shader type
    {
        khandle shader = state->material_standard_shader;

        if (!shader_system_bind_frame(shader)) {
            KERROR("Failed to bind frame frequency for standard material shader.");
            return false;
        }

        shader_system_uniform_set_by_location(shader, state->standard_material_locations.projection, &state->standard_frame_data.projection);
        shader_system_uniform_set_by_location_arrayed(shader, state->standard_material_locations.views, 0, &state->standard_frame_data.view);
        shader_system_uniform_set_by_location_arrayed(shader, state->standard_material_locations.view_positions, 0, &state->standard_frame_data.view_position);
        shader_system_uniform_set_by_location_arrayed(shader, state->standard_material_locations.views, 1, &state->standard_frame_data.inv_view);
        shader_system_uniform_set_by_location_arrayed(shader, state->standard_material_locations.view_positions, 1, &state->standard_frame_data.inv_view_position);
        shader_system_uniform_set_by_location(shader, state->standard_material_locations.render_mode, &state->standard_frame_data.render_mode);
        shader_system_uniform_set_by_location(shader, state->standard_material_locations.cascade_splits, &state->standard_frame_data.cascade_splits);

        // Light space for shadow mapping. Per cascade
        for (u32 i = 0; i < SHADOW_CASCADE_COUNT; ++i) {
            shader_system_uniform_set_by_location(shader, state->standard_material_locations.light_space_0 + i, &state->standard_frame_data.directional_light_spaces[i]);
        }

        // Global shader options.
        i32 use_pcf = (i32)renderer_pcf_enabled(engine_systems_get()->renderer_system);
        shader_system_uniform_set_by_location(shader, state->standard_material_locations.use_pcf, &use_pcf);

        shader_system_uniform_set_by_location(shader, state->standard_material_locations.bias, &state->standard_frame_data.bias);

        shader_system_uniform_set_by_location(shader, state->standard_material_locations.clipping_plane, &state->standard_frame_data.clipping_plane);

        // Apply/upload them to the GPU
        if (!shader_system_apply_per_frame(shader)) {
            KERROR("Failed to apply per-frame uniforms.");
            return false;
        }
    }

    // TODO: Water

    // TODO: Blended
}

b8 material_system_apply(material_system_state* state, material_instance* instance, u64 renderer_frame_number) {
    if (!state) {
        return false;
    }

    material_instance_data* instance_data = get_instance_data(state, *instance);
    if (!instance_data) {
        return false;
    }
    material_data* base_material = &state->materials[instance->material.handle_index];

    khandle shader;

    //TODO:DXS Shader system 重构过在进行 
}

material_instance material_system_get_default_unlit(material_system_state* state) {
    material_instance instance = { 0 };
    //FIXME: use kname instead
    u32 shader_id = shader_system_get_id("Shader.Unlit");
    // NOTE: No maps for this shader type.
    if (!shader_system_shader_per_draw_acquire(shader_id, 0, 0, &instance.per_draw_id)) {
        KASSERT_MSG(false, "Failed to acquire per-draw renderer resources for default Unlit material. Application cannot continue.");
    }
    instance.material = state->default_unlit_material;
    return instance;
}

material_instance material_system_get_default_phong(material_system_state* state) {
    material_instance instance = { 0 };
    // FIXME: use kname instead
    u32 shader_id = shader_system_get_id("Shader.Phong");
    // NOTE: No maps for this shader type.
    if (!shader_system_shader_per_draw_acquire(shader_id, 0, 0, &instance.per_draw_id)) {
        KASSERT_MSG(false, "Failed to acquire per-draw renderer resources for default Phong material. Application cannot continue.");
    }
    instance.material = state->default_phong_material;
    return instance;
}

material_instance material_system_get_default_pbr(material_system_state* state) {
    material_instance instance = { 0 };
    // FIXME: use kname instead
    u32 shader_id = shader_system_get_id("Shader.PBRMaterial");
    // NOTE: No maps for this shader type.
    if (!shader_system_shader_per_draw_acquire(shader_id, 0, 0, &instance.per_draw_id)) {
        KASSERT_MSG(false, "Failed to acquire per-draw renderer resources for default PBR material. Application cannot continue.");
    }
    instance.material = state->default_pbr_material;
    return instance;
}

material_instance material_system_get_default_layered_pbr(material_system_state* state) {
    material_instance instance = { 0 };
    // FIXME: use kname instead
    u32 shader_id = shader_system_get_id("Shader.LayeredPBRMaterial");
    // NOTE: No maps for this shader type.
    if (!shader_system_shader_per_draw_acquire(shader_id, 0, 0, &instance.per_draw_id)) {
        KASSERT_MSG(false, "Failed to acquire per-draw renderer resources for default LayeredPBR material. Application cannot continue.");
    }
    instance.material = state->default_layered_material;
    return instance;
}

void material_system_dump(material_system_state* state) {
    // FIXME: find a way to query this from the kresource system.
     //
     /* material_reference* refs = (material_reference*)state_ptr->registered_material_table.memory;
     for (u32 i = 0; i < state_ptr->registered_material_table.element_count; ++i) {
         material_reference* r = &refs[i];
         if (r->reference_count > 0 || r->handle != INVALID_ID) {
             KTRACE("Found material ref (handle/refCount): (%u/%u)", r->handle, r->reference_count);
             if (r->handle != INVALID_ID) {
                 KTRACE("Material name: %s", state_ptr->registered_materials[r->handle].name);
             }
         }
     } */
}

static b8 assign_map(material_system_state* state, kresource_texture_map* map, const material_map* config, kname material_name, const kresource_texture* default_tex) {
    map->filter_minify = config->filter_min;
    map->filter_magnify = config->filter_mag;
    map->repeat_u = config->repeat_u;
    map->repeat_v = config->repeat_v;
    map->repeat_w = config->repeat_w;
    map->mip_levels = 1;
    map->generation = INVALID_ID;

    if (config->texture_name && string_length(config->texture_name) > 0) {
        map->texture = texture_system_request(
            kname_create(config->texture_name),
            INVALID_KNAME,// Use the resource from the package where it is first found. TODO: configurable within material config - include material's package name here first.
            0, // no listener
            0);// no callback

        if (!map->texture) {
            // Use default texture instead if provided.
            if (default_tex) {
                KWARN("Failed to request material texture '%s'. Using default '%s'.", config->texture_name, kname_string_get(default_tex->base.name));
                map->texture = default_tex;
            }
            else {
                KERROR("Failed to request material texture '%s', and no default was provided.", config->texture_name);
                return false;
            }
        }
    }
    else {
        // This is done when a texture is not configured, as opposed to when it is configured and not found (above).
        map->texture = default_tex;
    }
    // Acquire texture map resources.
    if (!renderer_kresource_texture_map_resources_acquire(state->renderer, map)) {
        KERROR("Unable to acquire resources for texture map.");
        return false;
    }

    return true;
}

static b8 create_default_standard_material(material_system_state* state) {
    kresource_material_request_info request = { 0 };
    request.base.type = KRESOURCE_TYPE_MATERIAL;
    request.material_source_text = "\
version = 3\
type = \"standard\"\
\
albedo_texture = \"default_albedo\"\
normal_texture = \"default_normal\"\
mra_texture = \"default_mra\"\
emissive_texture = \"default_emissive\"\
emissive_intensity = 1.0\
has_transparency = false\
double_sided = false\
recieves_shadow = true\
casts_shadow = true\
normal_enabled = true\
ao_enabled = false\
emissive_enabled = false\
refraction_enabled = false\
use_vertex_colour_as_albedo = false";

    state->default_pbr_material = (kresource_material*)kresource_system_request(state->resource_system, kname_create("default"), (kresource_request_info*)&request);

    u32 shader_id = shader_system_get_id("Shader.PBRMaterial");
    kresource_material* m = state->default_pbr_material;

    kresource_texture_map* maps[PBR_MATERIAL_MAP_COUNT] = {
        &m->albedo_diffuse_map,
        &m->normal_map,
        &m->metallic_roughness_ao_map
        // TODO: emissive
    };

    // Acquire group resources.
    if (!shader_system_shader_group_acquire(shader_id, PBR_MATERIAL_MAP_COUNT, maps, &m->group_id)) {
        KERROR("Unable to acquire group resources for default PBR material.");
        return false;
    }

    return true;
}

static b8 create_default_multi_material(material_system_state* state) {
    kresource_material_request_info request = { 0 };
    request.base.type = KRESOURCE_TYPE_MATERIAL;
    // FIXME: figure out how the layers should look for this material type.
     //
    // TODO: Need to add "channel" property to each map separate from the name of
    // the map to indicate its usage.
    //
    // TODO: Layered materials will work somewhat differently than standard (see below
    // for example). Each "channel" will be represented by a arrayed texture whose number
    // of elements is equal to the number of layers in the material. This keeps the sampler
    // count low and also allows the loading of many textures for the terrain at once. The
    // mesh using this material should indicate the layer to be used at the vertex level (as
    // sampling this from an image limits to 4 layers (RGBA)).
    //
    // TODO: The size of all layers is determined by the channel_size_x/y in the material config,
    // OR by not specifying it and using the default of 1024. Texture data will be loaded into the
    // array by copying when the dimensions of the source texture match the channel_size_x/y, or by
    // blitting the texture onto the layer when it does not match. This gets around the requirement
    // of having all textures be the same size in an arrayed texture.
    //
    // TODO: This process will also be utilized by the metallic_roughness_ao_map (formerly "combined"),
    // but instead targeting a single channel of the target texture as opposed to a layer of it.
    request.material_source_text = "\
version = 3\
type = \"multi\"\
\
materials = [\
    \"default\"\
    \"default\"\
    \"default\"\
    \"default\"\
]";

    state->default_layered_material = (kresource_material*)kresource_system_request(state->resource_system, kname_create("default_layered"), (kresource_request_info*)&request);

    // TODO: change to layered material shader.
    u32 shader_id = shader_system_get_id("Shader.Builtin.Terrain");
    kresource_material* m = state->default_layered_material;

    // NOTE: This is an array that includes 3 maps (albedo, normal, met/roughness/ao) per layer.
    kresource_texture_map* maps[LAYERED_PBR_MATERIAL_MAP_COUNT] = { &m->layered_material_map };

    // Acquire group resources.
    if (!shader_system_shader_group_acquire(shader_id, LAYERED_PBR_MATERIAL_MAP_COUNT, maps, &m->group_id)) {
        KERROR("Unable to acquire group resources for default layered PBR material.");
        return false;
    }

    return true;
}

static void on_material_system_dump(console_command_context context) {
    material_system_dump(engine_systems_get()->material_system);
}

