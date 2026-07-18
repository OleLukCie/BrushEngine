#pragma once
#include <vulkan/vulkan.h>
#include <vector>

// 包裹函数调用版本：VK_CHECK(vkCreateXXX(...))
#define VK_CHECK(call) \
    do { \
        VkResult vk_result = (call); \
        if (vk_result != VK_SUCCESS) { \
            BE_LOG_ERROR("VK_CHECK failed: {} | {} at {}:{} | result={}", \
                #call, __FUNCTION__, __FILE__, __LINE__, static_cast<int>(vk_result)); \
            return false; \
        } \
    } while(0)

// 包裹返回值版本（兼容旧用法）：VK_CHECK_RESULT(someVkResult)
#define VK_CHECK_RESULT(result) \
    do { \
        VkResult vk_result = (result); \
        if (vk_result != VK_SUCCESS) { \
            BE_LOG_ERROR("VK_CHECK_RESULT failed at {}:{} | result={}", \
                __FILE__, __LINE__, static_cast<int>(vk_result)); \
            return false; \
        } \
    } while(0)

uint32_t findMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);