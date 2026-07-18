#define NOMINMAX
#include "base/log.h"
#include "vk_utils.h"
#include "../base/log.h"

uint32_t findMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    BE_LOG_ERROR("findMemoryType failed: no matching memory type for typeFilter={}, properties={}",
        typeFilter, properties);
    return 0;  // 返回 0 作为 fallback，但已记录错误
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        BE_LOG_ERROR("createShaderModule failed at {}:{}", __FILE__, __LINE__);
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}