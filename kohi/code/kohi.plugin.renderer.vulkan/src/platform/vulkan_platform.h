#pragma once

#include <defines.h>

struct kwindow;;
struct vulkan_context;

b8 vulkan_platform_create_vulkan_surface(struct vulkan_context* context,struct kwindow* window);
/*
   Appends the names of required extensions for this platform to 
   the names_darray. which should be created and passed in. ***字符数组的数组
*/
void vulkan_platform_get_required_extension_names(const char*** names_darray);