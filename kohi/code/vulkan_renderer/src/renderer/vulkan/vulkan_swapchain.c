#include "vulkan_swapchain.h"

#include "core/kmemory.h"
#include "core/kstring.h"
#include "core/logger.h"
#include "systems/texture_system.h"
#include "vulkan_device.h"
#include "vulkan_image.h"
#include "vulkan_utils.h"

static void create(vulkan_context* context, u32 width, u32 height, renderer_config_flags flags, vulkan_swapchain* swapchain);
static void destroy(vulkan_context* context, vulkan_swapchain* swapchain);

void vulkan_swapchain_create(vulkan_context* context, u32 width, u32 height, renderer_config_flags flags, vulkan_swapchain* out_swapchain) {
    // Simple create a new one
    create(context, width, height, flags, out_swapchain);
}

void vulkan_swapchain_recreate(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* swapchain) {
    // Destroy the old and create a new one
    destroy(context, swapchain);
    create(context, width, height, swapchain->flags, swapchain);
}

void vulkan_swapchain_destroy(vulkan_context* context, vulkan_swapchain* swapchain) {
    destroy(context, swapchain);
}

static void create(vulkan_context* context, u32 width, u32 height, renderer_config_flags flags, vulkan_swapchain* swapchain) {
    VkExtent2D swapchain_extent = {width, height};

    // Choose a swap surface format.
    b8 found = false;
    for (u32 i = 0; i < context->device.swapchain_support.format_count; ++i) {
        VkSurfaceFormatKHR format = context->device.swapchain_support.formats[i];
        // Preferred formats
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain->image_format = format;
            found = true;
            break;
        }
    }

    if (!found) {
        swapchain->image_format = context->device.swapchain_support.formats[0];
    }
    // FIFO and MAILBOX support vsync, IMMEDIATE does not.
    // TODO: vsync seems to hold up the game update for some reason.
    // It theoretically should be post-update and pre-render where that happens.
    swapchain->flags = flags;
    VkPresentModeKHR present_mode;
    if (flags & RENDERER_CONFIG_FLAG_VSYNC_ENABLED_BIT) {
        present_mode = VK_PRESENT_MODE_FIFO_KHR;
        // Only try for mailbox mode if not in power-saving mode.
        if ((flags & RENDERER_CONFIG_FLAG_POWER_SAVING_BIT) == 0) {
            for (u32 i = 0; i < context->device.swapchain_support.present_mode_count; ++i) {
                VkPresentModeKHR mode = context->device.swapchain_support.present_modes[i];
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    present_mode = mode;
                    break;
                }
            }
        }
    } else {
        present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    // Requery swapchain support.
    vulkan_device_query_swapchain_support(
        context->device.physical_device,
        context->surface,
        &context->device.swapchain_support);

    // Swapchain extent
    if (context->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX) {
        swapchain_extent = context->device.swapchain_support.capabilities.currentExtent;
    }

    // Clamp to the value allowed by the GPU.
    VkExtent2D min = context->device.swapchain_support.capabilities.minImageExtent;
    VkExtent2D max = context->device.swapchain_support.capabilities.maxImageExtent;
    swapchain_extent.width = KCLAMP(swapchain_extent.width, min.width, max.width);
    swapchain_extent.height = KCLAMP(swapchain_extent.height, min.height, max.height);

    u32 image_count = context->device.swapchain_support.capabilities.minImageCount + 1;
    if (context->device.swapchain_support.capabilities.maxImageCount > 0 && image_count > context->device.swapchain_support.capabilities.maxImageCount) {
        image_count = context->device.swapchain_support.capabilities.maxImageCount;
    }

    swapchain->max_frames_in_flight = image_count - 1;

    // Swapchain create info
    VkSwapchainCreateInfoKHR swapchain_create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchain_create_info.surface = context->surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = swapchain->image_format.format;
    swapchain_create_info.imageColorSpace = swapchain->image_format.colorSpace;
    swapchain_create_info.imageExtent = swapchain_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Setup the queue family indices
    if (context->device.graphics_queue_index != context->device.present_queue_index) {
        u32 queueFamilyIndices[] = {
            (u32)context->device.graphics_queue_index,
            (u32)context->device.present_queue_index};
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = 0;
    }

    swapchain_create_info.preTransform = context->device.swapchain_support.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = 0;

    VK_CHECK(vkCreateSwapchainKHR(context->device.logical_device, &swapchain_create_info, context->allocator, &swapchain->handle));

    // Start with a zero frame index.
    context->current_frame = 0;

    // Images
    swapchain->image_count = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device, swapchain->handle, &swapchain->image_count, 0));
    if (!swapchain->render_textures) {
        swapchain->render_textures = (texture*)kallocate(sizeof(texture) * swapchain->image_count, MEMORY_TAG_RENDERER);  // 指针只有8个字节
        // If creating the array, then the internal texture objects aren't created yet either.
        for (u32 i = 0; i < swapchain->image_count; ++i) {
            void* internal_data = kallocate(sizeof(vulkan_image), MEMORY_TAG_TEXTURE);

            char tex_name[38] = "__internal_vulkan_swapchain_image_0__";
            tex_name[34] = '0' + (char)i;

            texture_system_wrap_internal(
                tex_name,
                swapchain_extent.width,
                swapchain_extent.height,
                4,
                false,
                true,
                false,
                internal_data,
                &swapchain->render_textures[i]);
            if (!swapchain->render_textures[i].internal_data) {
                KFATAL("Failed to generate new swapchain image texture!");
                return;
            }
        }
    } else {
        for (u32 i = 0; i < swapchain->image_count; ++i) {
            // Just update the dimensions.
            texture_system_resize(&swapchain->render_textures[i], swapchain_extent.width, swapchain_extent.height, false);
        }
    }

    VkImage swapchain_images[32];
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device, swapchain->handle, &swapchain->image_count, swapchain_images));
    for (u32 i = 0; i < swapchain->image_count; ++i) {
        // Update the internal image for each
        vulkan_image* image = (vulkan_image*)swapchain->render_textures[i].internal_data;
        image->handle = swapchain_images[i];
        image->width = swapchain_extent.width;
        image->height = swapchain_extent.height;
    }

    // Views
    for (u32 i = 0; i < swapchain->image_count; ++i) {
        vulkan_image* image = (vulkan_image*)swapchain->render_textures[i].internal_data;

        VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view_info.image = image->handle;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = swapchain->image_format.format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(context->device.logical_device, &view_info, context->allocator, &image->view));
    }

    // Depth resources
    if (!vulkan_device_detect_depth_format(&context->device)) {
        context->device.depth_format = VK_FORMAT_UNDEFINED;
        KFATAL("Failed to find a supported format!");
    }

    // Allocate 深度RT
    if (!swapchain->depth_textures) {
        swapchain->depth_textures = (texture*)kallocate(sizeof(texture) * swapchain->image_count, MEMORY_TAG_RENDERER);
    }

    // Create depth/stencil images and its view
    for (u32 i = 0; i < context->swapchain.image_count; ++i) {
        char formatted_name[TEXTURE_NAME_MAX_LENGTH] = {0};
        string_format(formatted_name, "__kohi_default_depth_stencil_texture_%u", i);

        vulkan_image* image = kallocate(sizeof(vulkan_image), MEMORY_TAG_TEXTURE);
        vulkan_image_create(
            context,
            TEXTURE_TYPE_2D,
            swapchain_extent.width,
            swapchain_extent.height,
            1,
            context->device.depth_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            true,
            VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
            formatted_name,
            1,
            image);

        // Wrap it in a texture.
        texture_system_wrap_internal(
            formatted_name,
            swapchain_extent.width,
            swapchain_extent.height,
            context->device.depth_channel_count,
            false,
            true,
            false,
            image,
            &context->swapchain.depth_textures[i]);
    }

    KINFO("Swapchain created successfully.");
}

static void destroy(vulkan_context* context, vulkan_swapchain* swapchain) {
    vkDeviceWaitIdle(context->device.logical_device);
    for (u32 i = 0; i < context->swapchain.image_count; ++i) {
        vulkan_image_destroy(context, (vulkan_image*)swapchain->depth_textures[i].internal_data);
        swapchain->depth_textures[i].internal_data = 0;
    }

    // Only destroy the views ,not the images,since those are owned by the swachain and are thus
    // destroyed when it is
    for (u32 i = 0; i < swapchain->image_count; ++i) {
        vulkan_image* image = (vulkan_image*)swapchain->render_textures[i].internal_data;
        vkDestroyImageView(context->device.logical_device, image->view, context->allocator);
    }

    vkDestroySwapchainKHR(context->device.logical_device, swapchain->handle, context->allocator);
}
