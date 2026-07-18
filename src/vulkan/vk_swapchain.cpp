#define NOMINMAX
#include "base/log.h"
#include "vk_swapchain.h"
#include "vk_utils.h"
#include "vk_imageview.h"
#include "vk_renderpass.h"
#include "vk_commands.h"
#include "vk_sync.h"
#include "vk_canvas_tex.h"
#include "../engine/engine_state.h"

bool createSwapchain() {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_physicalDevice, g_surface, &caps);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_physicalDevice, g_surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_physicalDevice, g_surface, &formatCount, formats.data());

    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = f;
            break;
        }
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(g_physicalDevice, g_surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(g_physicalDevice, g_surface, &presentModeCount, presentModes.data());

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
        imageCount = caps.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = g_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = caps.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = caps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (g_graphicsQueueFamily != g_presentQueueFamily) {
        uint32_t queueFamilyIndices[] = { g_graphicsQueueFamily, g_presentQueueFamily };
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VK_CHECK(vkCreateSwapchainKHR(g_device, &createInfo, nullptr, &g_swapchain));

    vkGetSwapchainImagesKHR(g_device, g_swapchain, &imageCount, nullptr);
    g_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(g_device, g_swapchain, &imageCount, g_swapchainImages.data());

    g_imageCount = imageCount;
    g_maxFramesInFlight = g_imageCount;

    g_swapchainFormat = surfaceFormat.format;
    g_swapchainExtent = caps.currentExtent;

    return true;
}

void cleanupSwapchain() {
    for (auto framebuffer : g_framebuffers) {
        vkDestroyFramebuffer(g_device, framebuffer, nullptr);
    }
    g_framebuffers.clear();
    
    for (auto imageView : g_swapchainImageViews) {
        vkDestroyImageView(g_device, imageView, nullptr);
    }
    g_swapchainImageViews.clear();
    
    vkDestroySwapchainKHR(g_device, g_swapchain, nullptr);
    g_swapchain = VK_NULL_HANDLE;
}

bool recreateSwapchain() {
    vkDeviceWaitIdle(g_device);

    // ===== 1. 先销毁 canvas texture（会 RemoveTexture）=====
    destroyCanvasTexture();

    // ===== 2. 再 shutdown ImGui（释放内部所有 descriptor set）=====
    g_uiManager->shutdown();

    // 3. 释放旧的 command buffers
    if (!g_commandBuffers.empty()) {
        vkFreeCommandBuffers(g_device, g_commandPool,
            static_cast<uint32_t>(g_commandBuffers.size()), g_commandBuffers.data());
        g_commandBuffers.clear();
    }

    // 4. 释放旧的同步对象
    for (uint32_t i = 0; i < g_maxFramesInFlight; ++i) {
        vkDestroySemaphore(g_device, g_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(g_device, g_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(g_device, g_inFlightFences[i], nullptr);
    }
    g_imageAvailableSemaphores.clear();
    g_renderFinishedSemaphores.clear();
    g_inFlightFences.clear();

    cleanupSwapchain();

    // 5. 重新创建
    if (!createSwapchain()) return false;
    if (!createImageViews()) return false;
    if (!createFramebuffers()) return false;
    if (!createCommandBuffers()) return false;
    if (!createSyncObjects()) return false;

    // 6. 重新初始化 ImGui
    if (!g_uiManager->init(
        g_hwnd,
        g_instance,
        g_physicalDevice,
        g_device,
        g_graphicsQueueFamily,
        g_graphicsQueue,
        g_renderPass,
        static_cast<uint32_t>(g_swapchainImages.size()),
        static_cast<uint32_t>(g_swapchainImages.size())
    )) {
        BE_LOG_ERROR("ImGui re-initialization failed after swapchain recreate!");
        return false;
    }

    // 7. 重新创建 canvas texture（会 AddTexture）
    if (!createCanvasTexture(g_canvas->width(), g_canvas->height())) {
        BE_LOG_ERROR("Canvas texture recreation failed after swapchain recreate!");
        return false;
    }
    g_uiManager->setCanvasTexture(g_canvasTextureId);

    return true;
}