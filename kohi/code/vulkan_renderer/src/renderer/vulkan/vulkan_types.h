/**
 * @file vulkan_types.inl
 * @author Travis Vroman (travis@kohiengine.com)
 * @brief This file contains a collection fo Vulkan-specific types used
 * for the Vulkan backend.
 * @version 1.0
 * @date 2022-01-11
 *
 * @copyright Kohi Game Engine is Copyright (c) Travis Vroman 2021-2022
 *
 */

#pragma once

#include <vulkan/vulkan.h>

#include "containers/freelist.h"
#include "containers/hashtable.h"
#include "core/asserts.h"
#include "defines.h"
#include "renderer/renderer_types.h"
#include "vulkan/vulkan_core.h"

// Checks the given expression return value against VK_SUCCESS
#define VK_CHECK(expr) \
    {                  \
        KASSERT(expr == VK_SUCCESS)}

struct vulkan_context;

typedef struct vulkan_buffer {
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    b8 is_locked;
    VkDeviceMemory memory;
    /** @brief The memory requirements for this buffer. */
    VkMemoryRequirements memory_requirements;
    /** @brief The index of the memory used by the buffer. */
    i32 memory_index;
    u32 memory_property_flags;
} vulkan_buffer;

typedef struct vulkan_swapchain_support_info {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} vulkan_swapchain_support_info;

typedef enum vulkan_device_support_flag_bits {
  VULKAN_DEVICE_SUPPORT_FLAG_NONE_BIT = 0x00,

    /** @brief Indicates if the device supports native dynamic state (i.e. using Vulkan API >= 1.3). */
    VULKAN_DEVICE_SUPPORT_FLAG_NATIVE_DYNAMIC_STATE_BIT = 0x01,

    /** @brief Indicates if this device supports dynamic state. If not, the renderer will need to generate a separate pipeline per topology type. */
    VULKAN_DEVICE_SUPPORT_FLAG_DYNAMIC_STATE_BIT = 0x02,
    VULKAN_DEVICE_SUPPORT_FLAG_LINE_SMOOTH_RASTERISATION_BIT = 0x04,
} vulkan_device_support_flag_bits;

/** @brief Bitwise flags for device support. @see vulkan_device_support_flag_bits. */
typedef u32 vulkan_device_support_flags;

typedef struct vulkan_device {
    /** @brief The supported device-level api major version. */
    u32 api_major;
    /** @brief The supported device-level api minor version. */
    u32 api_minor;
    /** @brief The supported device-level api patch version. */
    u32 api_patch;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    vulkan_swapchain_support_info swapchain_support;
    i32 graphics_queue_index;
    i32 present_queue_index;
    i32 transfer_queue_index;
    b8 supports_device_local_host_visible;

    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;

    VkCommandPool graphics_command_pool;
    // 设备属性
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depth_format;
    /** @brief The chosen depth format's number of channels.*/
    u8 depth_channel_count;

    /** @brief Indicates support for various features. */
    vulkan_device_support_flags support_flags;
} vulkan_device;

typedef struct vulkan_image {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    /** @brief The GPU memory requirements for this image. */
    VkMemoryRequirements memory_requirements;
    /** @brief Memory property flags */
    VkMemoryPropertyFlags memory_flags;
    /** @brief The format of the image. */
    VkFormat format;
    u32 width;
    u32 height;
    /** @brief The name of the image. */
    char* name;
    /** The number of mipmaps to be generated for this image. Must always be at least 1. */
    u32 mip_levels;
} vulkan_image;

typedef enum vulkan_render_pass_state {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
} vulkan_render_pass_state;

/**
 * @brief A representation of the Vulkan renderpass.
 */
typedef struct vulkan_renderpass {
    VkRenderPass handle;

    /** @brief The depth clear value. */
    f32 depth;
    /** @brief The stencil clear value. */
    u32 stencil;

    vulkan_render_pass_state state;
} vulkan_renderpass;

typedef struct vulkan_swapchain {
    VkSurfaceFormatKHR image_format;
    u8 max_frames_in_flight;
    /** @brief Indicates various flags used for swapchain instantiation. */
    renderer_config_flags flags;
    VkSwapchainKHR handle;
    u32 image_count;
    /** @brief An array of to render targets, which contain swapchain images. */
    texture* render_textures;
    /** @brief An array of depth textures, one per frame. 深度纹理数组  */
    texture* depth_textures;

    /**
     * @brief Render targets used for on-screen rendering, one per frame.
     * The images contained in these are created and owned by the swapchain.
     * */
    render_target render_targets[3];
} vulkan_swapchain;

typedef enum vulkan_command_buffer_state {
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} vulkan_command_buffer_state;

typedef struct vulkan_command_buffer {
    VkCommandBuffer handle;

    // Command buffer state.
    vulkan_command_buffer_state state;
} vulkan_command_buffer;

typedef struct vulkan_shader_stage {
    VkShaderModuleCreateInfo create_info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
} vulkan_shader_stage;

typedef enum vulkan_topology_class {
    VULKAN_TOPOLOGY_CLASS_POINT = 0,
    VULKAN_TOPOLOGY_CLASS_LINE = 1,
    VULKAN_TOPOLOGY_CLASS_TRIANGLE = 2,
    VULKAN_TOPOLOGY_CLASS_MAX = VULKAN_TOPOLOGY_CLASS_TRIANGLE + 1
} vulkan_topology_class;

/**
 * @brief A configuration structure for Vulkan pipelines.
 */
typedef struct vulkan_pipeline_config {
    /** @brief The name of the pipeline. Used primarily for debugging purposes. */
    char* name;
    /** @brief A pointer to the renderpass to associate with the pipeline. */
    vulkan_renderpass* renderpass;
    /** @brief The stride of the vertex data to be used (ex: sizeof(vertex_3d)) */
    u32 stride;
    /** @brief The number of attributes. */
    u32 attribute_count;
    /** @brief An array of attributes. */
    VkVertexInputAttributeDescription* attributes;
    /** @brief The number of descriptor set layouts. */
    u32 descriptor_set_layout_count;
    /** @brief An array of descriptor set layouts. */
    VkDescriptorSetLayout* descriptor_set_layouts;
    /** @brief The number of stages (vertex, fragment, etc). */
    u32 stage_count;
    /** @brief An VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BITarray of stages. */
    VkPipelineShaderStageCreateInfo* stages;
    /** @brief The initial viewport configuration. */
    VkViewport viewport;
    /** @brief The initial scissor configuration. */
    VkRect2D scissor;
    /** @brief The face cull mode. */
    face_cull_mode cull_mode;
    /** @brief The shader flags used for creating the pipeline. */
    u32 shader_flags;
    /** @brief The number of push constant data ranges. */
    u32 push_constant_range_count;
    /** @brief An array of push constant data ranges. */
    range* push_constant_ranges;
    /** @brief Collection of topology types to be supported on this pipeline. */
    u32 topology_types;
    /** @brief The vertex winding order used to determine the front face of triangles. */
    renderer_winding winding;
} vulkan_pipeline_config;

typedef struct vulkan_pipeline {
    VkPipeline handle;
    VkPipelineLayout pipeline_layout;
    /** @brief Indicates the topology types used by this pipeline. See primitive_topology_type.*/
    u32 supported_topology_types;
} vulkan_pipeline;

// Max number of material instance
// TODO:make configurable
#define VULKAN_MAX_MATERIAL_COUNT 1024

// Max number of ui control instances
// TODO: make configurable
#define VULKAN_MAX_UI_COUNT 1024

/**
 * @brief Put some hard limits in place for the count of supported textures,
 * attributes, uniforms, etc. This is to maintain memory locality and avoid
 * dynamic allocations.
 */
/** @brief The maximum number of stages (such as vertex, fragment, compute, etc.) allowed. */
#define VULKAN_SHADER_MAX_STAGES 8
/** @brief The maximum number of textures allowed at the global level. */
#define VULKAN_SHADER_MAX_GLOBAL_TEXTURES 31
/** @brief The maximum number of textures allowed at the instance level. */
#define VULKAN_SHADER_MAX_INSTANCE_TEXTURES 31
/** @brief The maximum number of vertex input attributes allowed. */
#define VULKAN_SHADER_MAX_ATTRIBUTES 16
/**
 * @brief The maximum number of uniforms and samplers allowed at the
 * global, instance and local levels combined. It's probably more than
 * will ever be needed.
 */
#define VULKAN_SHADER_MAX_UNIFORMS 128

/** @brief The maximum number of bindings per descriptor set. */
#define VULKAN_SHADER_MAX_BINDINGS 2
/** @brief The maximum number of push constant ranges for a shader. */
#define VULKAN_SHADER_MAX_PUSH_CONST_RANGES 32

/**
 * @brief Configuration for a shader stage, such as vertex or fragment.
 */
typedef struct vulkan_shader_stage_config {
    /** @brief The shader stage bit flag. */
    VkShaderStageFlagBits stage;
    /** @brief The shader file name. */
    char file_name[255];
} vulkan_shader_stage_config;
/**
 * @brief The configuration for a descriptor set.
 */
typedef struct vulkan_descriptor_set_config {
    /** @brief The number of bindings in this set. */
    u8 binding_count;
    /** @brief An array of binding layouts for this set. */
    VkDescriptorSetLayoutBinding bindings[VULKAN_SHADER_MAX_BINDINGS];
    /** @brief The index of the sampler binding. */
    u8 sampler_binding_index;
} vulkan_descriptor_set_config;

/** @brief Internal shader configuration generated by vulkan_shader_create(). */
typedef struct vulkan_shader_config {
    /** @brief The number of shader stages in this shader. */
    u8 stage_count;
    /** @brief  The configuration for every stage of this shader. */
    vulkan_shader_stage_config stages[VULKAN_SHADER_MAX_STAGES];
    /** @brief An array of descriptor pool sizes. */
    VkDescriptorPoolSize pool_sizes[2];
    /**
     * @brief The max number of descriptor sets that can be allocated from this shader.
     * Should typically be a decently high number.
     */
    u16 max_descriptor_set_count;
    /**
     * @brief The total number of descriptor sets configured for this shader.
     * Is 1 if only using global uniforms/samplers; otherwise 2.
     */
    u8 descriptor_set_count;
    /** @brief Descriptor sets, max of 2. Index 0=global, 1=instance */
    vulkan_descriptor_set_config descriptor_sets[2];
    /** @brief An array of attribute descriptions for this shader. */
    VkVertexInputAttributeDescription attributes[VULKAN_SHADER_MAX_ATTRIBUTES];

    /** @brief Face culling mode, provided by the front end. */
    face_cull_mode cull_mode;
} vulkan_shader_config;

/**
 * @brief Represents a state for a given descriptor. This is used
 * to determine when a descriptor needs updating. There is a state
 * per frame (with a max of 3).
 */
typedef struct vulkan_descriptor_state {
    /** @brief The descriptor generation, per frame. */
    u8 generations[3];
    /** @brief The identifier, per frame. Typically used for texture ids. */
    u32 ids[3];
} vulkan_descriptor_state;

/**
 * @brief Represents the state for a descriptor set. This is used to track
 * generations and updates, potentially for optimization via skipping
 * sets which do not need updating.
 */
typedef struct vulkan_shader_descriptor_set_state {
    /** @brief The descriptor sets for this instance, one per frame. */
    VkDescriptorSet descriptor_sets[3];

    /** @brief A descriptor state per descriptor, which in turn handles frames. Count is managed in shader config. */
    vulkan_descriptor_state descriptor_states[VULKAN_SHADER_MAX_BINDINGS];
} vulkan_shader_descriptor_set_state;

/**
 * @brief The instance-level state for a shader.
 */
typedef struct vulkan_shader_instance_state {
    /** @brief The instance id. INVALID_ID if not used. */
    u32 id;
    /** @brief The offset in bytes in the instance uniform buffer. */
    u64 offset;
    /** @brief  A state for the descriptor set. */
    vulkan_shader_descriptor_set_state descriptor_set_state;
    /**
     * @brief Instance texture map  pointers, which are used during rendering. These
     * are set by calls to set_sampler.
     */
    struct texture_map** instance_texture_maps;

} vulkan_shader_instance_state;

/**
 * @brief Represents a generic Vulkan shader. This uses a set of inputs
 * and parameters, as well as the shader programs contained in SPIR-V
 * files to construct a shader for use in rendering.
 */
typedef struct vulkan_shader {
    /** @brief The block of memory mapped to the uniform buffer. */
    void* mapped_uniform_buffer_block;
    /** @brief The shader identifier. */
    u32 id;
    /** @brief The configuration of the shader generated by vulkan_create_shader(). */
    vulkan_shader_config config;
    /** @brief A pointer to the renderpass to be used with this shader. */
    vulkan_renderpass* renderpass;
    /** @brief An array of stages (such as vertex and fragment) for this shader. Count is located in config.*/
    vulkan_shader_stage stages[VULKAN_SHADER_MAX_STAGES];
    /** @brief The descriptor pool used for this shader. */
    VkDescriptorPool descriptor_pool;
    /** @brief Descriptor set layouts, max of 2. Index 0=global, 1=instance. */
    VkDescriptorSetLayout descriptor_set_layouts[2];
    /** @brief Global descriptor sets, one per frame. */
    VkDescriptorSet global_descriptor_sets[3];
    /** @brief The uniform buffer used by this shader. */
    renderbuffer uniform_buffer;

    /** @brief An array of pointers to pipelines associated with this shader. */
    vulkan_pipeline** pipelines;
    /** @brief The currently bound pipeline index. */
    u8 bound_pipeline_index;
    /** @brief The currently-selected topology. */
    VkPrimitiveTopology current_topology;

    /** @brief The instance states for all instances. @todo TODO: make dynamic */
    u32 instance_count;
    vulkan_shader_instance_state instance_states[VULKAN_MAX_MATERIAL_COUNT];

    /** @brief The number of global non-sampler uniforms. */
    u8 global_uniform_count;
    /** @brief The number of global sampler uniforms. */
    u8 global_uniform_sampler_count;
    /** @brief The number of instance non-sampler uniforms. */
    u8 instance_uniform_count;
    /** @brief The number of instance sampler uniforms. */
    u8 instance_uniform_sampler_count;
    /** @brief The number of local non-sampler uniforms. */
    u8 local_uniform_count;
} vulkan_shader;

// Forward declare shader compiler.
struct shaderc_comppiler;

/**
 * @brief The overall Vulkan context for the backend. Holds and maintains
 * global renderer backend state, Vulkan instance, etc.
 */
typedef struct vulkan_context {
    /** @brief The instance-level api major version. */
    u32 api_major;

    /** @brief The instance-level api minor version. */
    u32 api_minor;

    /** @brief The instance-level api patch version. */
    u32 api_patch;
    // The framebuffer current width.
    u32 framebuffer_width;

    // The framebuffer current height.
    u32 framebuffer_height;

    // Current generation of framebuffer size.If it does not match framebuffer_size_last_generation,
    // a new one should be generated.
    u64 framebuffer_size_generation;

    // The generation of the framebuffer when it was last created. Set to framebuffer_size_generation
    // when updated
    u64 framebuffer_size_last_generation;

    /** @brief The viewport rectangle. */
    vec4 viewport_rect;

    /** @brief The scissor rectangle. */
    vec4 scissor_rect;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;
#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;
    /** @brief The function pointer to set debug object names. */
    PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT;

    /** @brief The function pointer to set free-form debug object tag data. */
    PFN_vkSetDebugUtilsObjectTagEXT pfnSetDebugUtilsObjectTagEXT;

    PFN_vkCmdBeginDebugUtilsLabelEXT pfnCmdBeginDebugUtilsLabelEXT;
    PFN_vkCmdEndDebugUtilsLabelEXT pfnCmdEndDebugUtilsLabelEXT;

#endif

    vulkan_device device;

    vulkan_swapchain swapchain;

    // darray
    vulkan_command_buffer* graphics_command_buffers;

    // darray
    VkSemaphore* image_available_semaphores;

    // darray
    VkSemaphore* queue_complete_semaphores;

    u32 in_flight_fence_count;
    VkFence in_flight_fences[2];

    u32 image_index;
    u32 current_frame;

    b8 recreating_swapchain;

    b8 render_flag_changed;

    /** @brief Render targets used for world rendering. @note One per frame. */
    render_target world_render_targets[3];

    /** @brief Indicates if multi-threading is supported by this device. */
    b8 multithreading_enabled;

    /**
     * @brief A function pointer to find a memory index of the given type and with the given properties.
     * @param context A pointer to the renderer context.
     * @param type_filter The types of memory to search for.
     * @param property_flags The required properties which must be present.
     * @returns The index of the found memory type. Returns -1 if not found.
     */
    i32 (*find_memory_index)(struct vulkan_context* context, u32 type_filter, u32 property_flags);

    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT;
    PFN_vkCmdSetFrontFaceEXT vkCmdSetFrontFaceEXT;
    PFN_vkCmdSetStencilTestEnable vkCmdSetStencilTestEnableEXT;
    PFN_vkCmdSetDepthTestEnableEXT vkCmdSetDepthTestEnableEXT;
    PFN_vkCmdSetStencilOpEXT vkCmdSetStencilOpEXT;

    /** @brief A pointer to the currently bound shader. */
    struct shader* bound_shader;

    /** @brief A resusable staging buffer to transfer data from a resource to a GPU-only buffer. */
    renderbuffer staging;

    /**
     * Used for dynamic compilation of vulkan shaders (using the shaderc lib.)
     */
    struct shaderc_compiler* shader_compiler;
} vulkan_context;
