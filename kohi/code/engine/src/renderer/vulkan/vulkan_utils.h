#pragma once
#include "vulkan_types.inl"

/*
  Returns the string representation of result.
  @param result: The result to get the string representation of.
  get_extended Indicates whether to also returen an extended result.
  The error code and/or extended error message in string form.Defaults to success for unknow result types
*/
const char* vulkan_result_to_string(VkResult result, b8 get_extended);

/*
  Inticates if the passed result is a success or an error as defined by the vulkan spec.
  True if success; otherwise false. defaults to true for unknow result types
*/
b8 vulkan_result_is_success(VkResult result);