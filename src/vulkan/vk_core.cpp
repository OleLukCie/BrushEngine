#define NOMINMAX
#include "base/log.h"
#include "vk_core.h"
#include "vk_utils.h"
#include "vk_instance.h"
#include "vk_surface.h"
#include "vk_device.h"
#include "vk_swapchain.h"
#include "vk_imageview.h"
#include "vk_renderpass.h"
#include "vk_commands.h"
#include "vk_sync.h"
#include "vk_canvas_tex.h"

#include "../engine/engine_state.h"
#include "../base/log.h"

// ===== Validation Layer 全局定义 =====
bool g_enableValidationLayers = true;  // Debug 模式开，Release 关
const std::vector<const char*> g_validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// ===== Vulkan Debug Callback =====
static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    (void)pUserData;
    std::string_view msg(pCallbackData->pMessage);

    // ========== 过滤 Vulkan Loader 的冗余 GENERAL 日志 ==========
    if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        // 过滤掉 loader 的 manifest/registry/callstack 内部日志
        if (msg.find("loader_get_json") != std::string_view::npos ||
            msg.find("windows_get_") != std::string_view::npos ||
            msg.find("windows_add_") != std::string_view::npos ||
            msg.find("Found manifest file") != std::string_view::npos ||
            msg.find("Found ICD manifest") != std::string_view::npos ||
            msg.find("Located json file") != std::string_view::npos ||
            msg.find("Checking for") != std::string_view::npos ||
            msg.find("layer callstack setup") != std::string_view::npos ||
            msg.find("Insert instance layer") != std::string_view::npos ||
            msg.find("Insert device layer") != std::string_view::npos ||
            msg.find("Insert") != std::string_view::npos) {  // 更宽泛的过滤
            return VK_FALSE; // 不记录，直接返回
        }
    }

    // 把 shader validation cache 的 info 也过滤掉
    if (msg.find("Cannot open shader validation cache") != std::string_view::npos) {
        return VK_FALSE;
    }

    const char* typeStr = "";
    switch (messageType) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:    typeStr = "[GENERAL] "; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: typeStr = "[VALIDATION] "; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:typeStr = "[PERF] "; break;
        default: typeStr = "[UNKNOWN] "; break;
    }

    std::string fullMsg = std::string(typeStr) + pCallbackData->pMessage;

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        BE_LOG_ERROR("Vulkan: {}", fullMsg);
    }
    else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        BE_LOG_WARN("Vulkan: {}", fullMsg);
    }
    else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        BE_LOG_INFO("Vulkan: {}", fullMsg);
    }
    else {
        BE_LOG_DEBUG("Vulkan: {}", fullMsg);
    }

    return VK_FALSE;
}

// 加载 vkCreateDebugUtilsMessengerEXT
static VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

// 全局 debug messenger
static VkDebugUtilsMessengerEXT g_debugMessenger = VK_NULL_HANDLE;

static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = vulkanDebugCallback;
    createInfo.pUserData = nullptr;
}

bool initVulkan() {
    BE_LOG_INFO("Vulkan initialization started");

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    populateDebugMessengerCreateInfo(debugCreateInfo);

    if (!createInstance(&debugCreateInfo)) {
        BE_LOG_FATAL("Failed to create Vulkan instance");
        return false;
    }

    // 创建 Debug Messenger（必须在 instance 创建之后）
    if (g_enableValidationLayers) {
        VkResult result = createDebugUtilsMessengerEXT(g_instance, &debugCreateInfo, nullptr, &g_debugMessenger);
        if (result != VK_SUCCESS) {
            BE_LOG_WARN("Failed to create Vulkan debug messenger: {}", static_cast<int>(result));
        } else {
            BE_LOG_INFO("Vulkan debug messenger created");
        }
    }

    if (!createSurface()) {
        BE_LOG_FATAL("Failed to create Vulkan surface");
        return false;
    }
    if (!pickPhysicalDevice()) {
        BE_LOG_FATAL("Failed to pick physical device");
        return false;
    }
    if (!createLogicalDevice()) {
        BE_LOG_FATAL("Failed to create logical device");
        return false;
    }
    if (!createSwapchain()) {
        BE_LOG_FATAL("Failed to create swapchain");
        return false;
    }
    if (!createImageViews()) {
        BE_LOG_FATAL("Failed to create image views");
        return false;
    }
    if (!createRenderPass()) {
        BE_LOG_FATAL("Failed to create render pass");
        return false;
    }
    if (!createFramebuffers()) {
        BE_LOG_FATAL("Failed to create framebuffers");
        return false;
    }
    if (!createCommandPool()) {
        BE_LOG_FATAL("Failed to create command pool");
        return false;
    }
    if (!createCommandBuffers()) {
        BE_LOG_FATAL("Failed to create command buffers");
        return false;
    }
    if (!createSyncObjects()) {
        BE_LOG_FATAL("Failed to create sync objects");
        return false;
    }

    BE_LOG_INFO("Vulkan initialization completed successfully");
    return true;
}

void cleanupVulkan() {
    BE_LOG_INFO("Vulkan cleanup started");
    vkDeviceWaitIdle(g_device);

    // 先销毁 debug messenger
    if (g_debugMessenger != VK_NULL_HANDLE) {
        destroyDebugUtilsMessengerEXT(g_instance, g_debugMessenger, nullptr);
        g_debugMessenger = VK_NULL_HANDLE;
        BE_LOG_DEBUG("Vulkan debug messenger destroyed");
    }

    // 销毁 canvas texture（在 ImGui shutdown 之后、command pool 之前）
    destroyCanvasTexture();

    // 销毁同步对象
    for (uint32_t i = 0; i < g_maxFramesInFlight; ++i) {
        if (g_imageAvailableSemaphores[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(g_device, g_imageAvailableSemaphores[i], nullptr);
        if (g_renderFinishedSemaphores[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(g_device, g_renderFinishedSemaphores[i], nullptr);
        if (g_inFlightFences[i] != VK_NULL_HANDLE)
            vkDestroyFence(g_device, g_inFlightFences[i], nullptr);
    }
    g_imageAvailableSemaphores.clear();
    g_renderFinishedSemaphores.clear();
    g_inFlightFences.clear();

    // 清空 image-in-flight 追踪
    g_imagesInFlight.clear();

    // 销毁 command pool（会隐式释放所有 command buffer）
    if (g_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(g_device, g_commandPool, nullptr);
        g_commandPool = VK_NULL_HANDLE;
    }

    // 清理 swapchain 相关
    cleanupSwapchain();

    // 销毁 render pass
    if (g_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(g_device, g_renderPass, nullptr);
        g_renderPass = VK_NULL_HANDLE;
    }

    // 销毁 device
    if (g_device != VK_NULL_HANDLE) {
        vkDestroyDevice(g_device, nullptr);
        g_device = VK_NULL_HANDLE;
    }

    // 销毁 surface
    if (g_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(g_instance, g_surface, nullptr);
        g_surface = VK_NULL_HANDLE;
    }

    // 销毁 instance
    if (g_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(g_instance, nullptr);
        g_instance = VK_NULL_HANDLE;
    }

    BE_LOG_INFO("Vulkan cleanup completed");
}