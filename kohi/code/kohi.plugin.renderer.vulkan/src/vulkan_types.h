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

#include "kdebug/kassert.h"
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
    /** @brief The handle to the internal image object. */
    VkImage handle;
    /** @brief The memory used by the image. */
    VkDeviceMemory memory;
    /** @brief The image creation info. */
    VkImageCreateInfo image_create_info;

    /** @brief The view for the image, which is used to access the image. */
    VkImageView view;
    VkImageSubresourceRange view_subresource_range;
    VkImageViewCreateInfo view_create_info;
    /** @brief If there are multiple layers, one view per layer exists here. ImageView 数组 */
    VkImageView* layer_views;
    VkImageSubresourceRange* layer_view_subresource_ranges;
    VkImageViewCreateInfo* layer_view_create_infos;
    /** @brief The GPU memory requirements for this image. */
    VkMemoryRequirements memory_requirements;
    /** @brief Memory property flags */
    VkMemoryPropertyFlags memory_flags;
    /** @brief The format of the image. */
    VkFormat format;
    /** @brief The image width. */
    u32 width;
    /** @brief The image height. */
    u32 height;
    /** @brief The number of layers in this image. */
    u16 layer_count;
    /** @brief The name of the image. */
    char* name;
    /** The number of mipmaps to be generated for this image. Must always be at least 1. */
    u32 mip_levels;
    b8 has_view;
} vulkan_image;

// Struct definition for renderer-specific texture data.
typedef struct texture_internal_data {
    // Number of vulkan_images in the array. This is typically 1 unless the texture
   // requires the frame_count to be taken into account.
    u32 image_count;
    // Array of images.
    vulkan_image* images;
}texture_internal_data;

// Struct definition for renderer-specific framebuffer data.
typedef struct framebuffer_internal_data {
    // The number of VkFramebuffers in the array. Typically 1 unless the attachment
       // requires the frame_count to be taken into account.
    u32 framebuffer_count;
    // Arrat of framebuffers.
    VkFramebuffer* framebuffers;
}framebuffer_internal_data;

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
    /** @brief The internal renderpass handle. */
    VkRenderPass handle;
    /** @brief Indicates renderpass state. */
    vulkan_render_pass_state state;

    // darray
    VkClearValue* clear_values;
    /* u32 clear_value_count; */
} vulkan_renderpass;

typedef struct vulkan_swapchain {
    /** @brief The swapchain image format. */
    VkSurfaceFormatKHR image_format;
    /**
  * @brief The maximum number of "images in flight" (images simultaneously being rendered to).
  * Typically one less than the total number of images available.
  */
    u8 max_frames_in_flight;

    /** @brief Indicates various flags used for swapchain instantiation. */
    renderer_config_flags flags;

    /** @brief The swapchain internal handle. */
    VkSwapchainKHR handle;
    /** @brief The number of swapchain images. */
    u32 image_count;

    /** @brief Track the owning window in case something is needed from it. */
    struct kwindow* owning_window;
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

    u32 colour_attachment_count;
    VkFormat* colour_attachment_formats;
    VkFormat depth_attachment_format;
    VkFormat stencil_attachment_format;
} vulkan_pipeline_config;

typedef struct vulkan_pipeline {
    VkPipeline handle;
    VkPipelineLayout pipeline_layout;
    /** @brief Indicates the topology types used by this pipeline. See primitive_topology_type.*/
    u32 supported_topology_types;
} vulkan_pipeline;

/**
 * @brief Put some hard limits in place for the count of supported textures,
 * attributes, uniforms, etc. This is to maintain memory locality and avoid
 * dynamic allocations.
 */
 /** @brief The maximum number of stages (such as vertex, fragment, compute, etc.) allowed. */
#define VULKAN_SHADER_MAX_STAGES 8
/** @brief The maximum number of textures bindings allowed at once. */
#define VULKAN_SHADER_MAX_TEXTURE_BINDINGS 31
/** @brief The maximum number of vertex input attributes allowed. */
#define VULKAN_SHADER_MAX_ATTRIBUTES 16
/**
 * @brief The maximum number of uniforms and samplers allowed at the
 * global, instance and local levels combined. It's probably more than
 * will ever be needed.
 */
#define VULKAN_SHADER_MAX_UNIFORMS 128

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
    VkDescriptorSetLayoutBinding* bindings;
    /** @brief The start index of the sampler bindings. */
    u8 sampler_binding_index_start;
} vulkan_descriptor_set_config;

/**
 * @brief Represents a state for a given descriptor. This is used
 * to determine when a descriptor needs updating. There is a state
 * per frame (with a max of 3).
 */
typedef struct vulkan_descriptor_state {
    /** @brief The descriptor generation, per swapchain image. */
    u8* generations;
    /** @brief The identifier, per swapchain image. Typically used for texture ids. */
    u32* ids;
    /** @brief The frame number this descriptor was last updated on, per swapchain image. */
    u64* frame_numbers;
} vulkan_descriptor_state;

typedef struct vulkan_uniform_sampler_state {
    struct shader_uniform* uniform;
    /**
     * @brief Instance texture map  pointers, which are used during rendering. These
     * are set by calls to set_sampler.
     */
    struct texture_map** uniform_texture_maps;
    /** @brief A descriptor state per descriptor, which in turn handles frames.
     * Count is managed in shader config.
     * */
    vulkan_descriptor_state* descriptor_states;
} vulkan_uniform_sampler_state;

/**
 * @brief The instance-level state for a shader.
 */
typedef struct vulkan_shader_instance_state {
    /** @brief The instance id. INVALID_ID if not used. */
    u32 id;
    /** @brief The offset in bytes in the instance uniform buffer. */
    u64 offset;

    /** @brief The descriptor sets for this instance, one per frame. */
    VkDescriptorSet* descriptor_sets;

    // UBO descriptor
    vulkan_descriptor_state ubo_descriptor_state;

    // A mapping of sampler uniforms to descriptors and texture maps.
    vulkan_uniform_sampler_state* sampler_uniforms;
} vulkan_shader_instance_state;

/**
 * @brief Represents a generic Vulkan shader. This uses a set of inputs
 * and parameters, as well as the shader programs contained in SPIR-V
 * files to construct a shader for use in rendering.
 */
typedef struct vulkan_shader {
    /** @brief The block of memory mapped to the uniform buffer. */
    void** mapped_uniform_buffer_blocks;
    /** @brief The block of memory used for push constants,128B */
    void* local_push_constant_block;
    /** @brief The shader identifier. */
    u32 id;
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

    u32 max_instances;

    /** @brief The number of shader stages in this shader. */
    u8 stage_count;

    /** @brief An array of stages (such as vertex and fragment) for this shader. Count is located in config.*/
    vulkan_shader_stage stages[VULKAN_SHADER_MAX_STAGES];

    u32 pool_size_count;

    /** @brief An array of descriptor pool sizes. */
    VkDescriptorPoolSize pool_sizes[2];

    /** @brief The descriptor pool used for this shader. */
    VkDescriptorPool descriptor_pool;

    /** @brief Descriptor set layouts, max of 2. Index 0=global, 1=instance. */
    VkDescriptorSetLayout descriptor_set_layouts[2];

     /** @brief Global descriptor sets, one per swapchain image. */
    VkDescriptorSet* global_descriptor_sets;

    // UBO descriptor
    vulkan_descriptor_state global_ubo_descriptor_state;

    // A mapping of sampler uniforms to descriptors and texture maps.
    vulkan_uniform_sampler_state* global_sampler_uniforms;

     /** @brief The uniform buffers used by this shader, one per swapchain image. */
    renderbuffer* uniform_buffers;
    u32 uniform_buffer_count;

    /** @brief An array of pointers to pipelines associated with this shader. */
    vulkan_pipeline** pipelines;
    /** @brief An Array of pointer to wireframe pipelines associated with this shader*/
    vulkan_pipeline** wireframe_pipelines;

    /** @brief The currently bound pipeline index. */
    u8 bound_pipeline_index;
    /** @brief The currently-selected topology. */
    VkPrimitiveTopology current_topology;

    vulkan_shader_instance_state* instance_states;
}vulkan_shader;

// Forward declare shader compiler.
struct shaderc_comppiler;


/**
 * @brief The Vulkan-specific backend window state.
 *
 * This owns all resources associated with the window (i.e swapchain)
 * and anything tied to it or max_frames_in_flight (sync objects, staging
 * buffer, command buffers, etc.).
 */
typedef struct kwindow_renderer_backend_state {
    /** @brief The internal Vulkan surface for the window to be drawn to. */
    VkSurfaceKHR surface;
    /** @brief The swapchain. */
    vulkan_swapchain swapchain;
    /** @brief The current image index. */
    u32 image_index;
    /** @brief The current frame index ( % by max_frames_in_flight). */
    u32 current_frame;

    /** @brief Indicates if the swapchain is currently being recreated. */
    b8 recreating_swapchain;
    /** @brief The graphics command buffers, one per swapchain image. */
    vulkan_command_buffer* graphics_command_buffers;

    /** @brief The semaphores used to indicate image availability, one per frame in flight. */
    VkSemaphore* image_available_semaphores;
    /** @brief The semaphores used to indicate queue availability, one per frame in flight. */
    VkSemaphore* queue_complete_semaphores;

    /**
    * @brief The in-flight fences, used to indicate to the application when a frame is
    * busy/ready. One per frame in flight.
    */
    VkFence* in_flight_fences;
    /** @brief Resusable staging buffers (one per frame in flight) to transfer data from a resource to a GPU-only buffer. */
    renderbuffer* staging;

    u64 framebuffer_size_generation;
    u64 framebuffer_previous_size_generation;

    u8 skip_frames;
}kwindow_renderer_backend_state;
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

    renderer_config_flags flags;

    /** @brief The currently cached colour buffer clear value. */
    VkClearColorValue colour_clear_value;
    /** @brief The currently cached depth/stencil buffer clear value. */
    VkClearDepthStencilValue depth_stencil_clear_value;

    /** @brief The viewport rectangle. */
    vec4 viewport_rect;

    /** @brief The scissor rectangle. */
    vec4 scissor_rect;

    VkInstance instance;
    VkAllocationCallbacks* allocator;

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

    /** @brief A pointer to the current window whose resources should be used as default to render to. */
    struct kwindow* current_window;
    b8 render_flag_changed;

    b8 validation_enabled;

    /** @brief Indicates if multi-threading is supported by this device. */
    b8 multithreading_enabled;

    /** @brief Collection of samplers. darray */
    VkSampler* samplers;

    /**
     * @brief A function pointer to find a memory index of the given type and with the given properties.
     * @param context A pointer to the renderer context.
     * @param type_filter The types of memory to search for.
     * @param property_flags The required properties which must be present.
     * @returns The index of the found memory type. Returns -1 if not found.
     */
    i32(*find_memory_index)(struct vulkan_context* context, u32 type_filter, u32 property_flags);

    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT;
    PFN_vkCmdSetFrontFaceEXT vkCmdSetFrontFaceEXT;
    PFN_vkCmdSetStencilTestEnable vkCmdSetStencilTestEnableEXT;
    PFN_vkCmdSetDepthTestEnableEXT vkCmdSetDepthTestEnableEXT;
    PFN_vkCmdSetDepthWriteEnable vkCmdSetDepthWriteEnableEXT;
    PFN_vkCmdSetStencilOpEXT vkCmdSetStencilOpEXT;

    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR;
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR;

    /** @brief A pointer to the currently bound shader. */
    struct shader* bound_shader;

    /**
     * Used for dynamic compilation of vulkan shaders (using the shaderc lib.)
     */
    struct shaderc_compiler* shader_compiler;
} vulkan_context;
