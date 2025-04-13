#include "shader/shader.hpp"
#include <fstream>
#include <stdexcept>

namespace Shader {

// Load shader code from file
std::string loadShaderFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filename);
    }
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::string buffer(fileSize, ' ');
    
    file.seekg(0);
    file.read(&buffer[0], fileSize);
    
    file.close();
    return buffer;
}

// Parse combined shader file (.shader) to get vertex and fragment parts
std::pair<std::string, std::string> parseCombinedShader(const std::string& filename) {
    std::string combinedCode = loadShaderFile(filename);
    
    // Look for markers that separate vertex and fragment shaders
    const std::string vertexMarker = "#shader vertex";
    const std::string fragmentMarker = "#shader fragment";
    
    size_t vertexPos = combinedCode.find(vertexMarker);
    size_t fragmentPos = combinedCode.find(fragmentMarker);
    
    if (vertexPos == std::string::npos || fragmentPos == std::string::npos) {
        throw std::runtime_error("Invalid shader format in: " + filename);
    }
    
    // Extract vertex shader (after vertex marker, before fragment marker)
    std::string vertexCode;
    if (vertexPos < fragmentPos) {
        vertexPos += vertexMarker.length();
        vertexCode = combinedCode.substr(vertexPos, fragmentPos - vertexPos);
    } else {
        vertexPos += vertexMarker.length();
        vertexCode = combinedCode.substr(vertexPos);
    }
    
    // Extract fragment shader (after fragment marker)
    fragmentPos += fragmentMarker.length();
    std::string fragmentCode = combinedCode.substr(fragmentPos);
    
    return {vertexCode, fragmentCode};
}

// Create shader module from string
VkShaderModule createShaderModule(VkDevice device, const std::string& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }
    
    return shaderModule;
}

} // namespace Shader