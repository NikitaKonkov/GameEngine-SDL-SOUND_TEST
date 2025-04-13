#pragma once

#include <string>
#include <utility>
#include <vulkan/vulkan.h>

namespace Shader {

// Load shader code from file
std::string loadShaderFile(const std::string& filename);

// Parse combined shader file (.shader) to get vertex and fragment parts
std::pair<std::string, std::string> parseCombinedShader(const std::string& filename);

// Create shader module from string
VkShaderModule createShaderModule(VkDevice device, const std::string& code);

} // namespace Shader