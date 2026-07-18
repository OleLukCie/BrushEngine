#pragma once

#include <vulkan/vulkan.h>
#include <vector>

// ===== 全局 Vulkan 对象 =====
extern VkInstance g_instance;
extern VkSurfaceKHR g_surface;
extern VkPhysicalDevice g_physicalDevice;
extern VkDevice g_device;
extern VkQueue g_graphicsQueue;
extern uint32_t g_graphicsQueueFamily;
extern VkRenderPass g_renderPass;
extern VkCommandPool g_commandPool;
extern std::vector<VkCommandBuffer> g_commandBuffers;
extern VkSwapchainKHR g_swapchain;
extern std::vector<VkImage> g_swapchainImages;
extern std::vector<VkImageView> g_swapchainImageViews;
extern std::vector<VkFramebuffer> g_framebuffers;
extern uint32_t g_swapchainImageCount;
extern uint32_t g_maxFramesInFlight;
extern std::vector<VkSemaphore> g_imageAvailableSemaphores;
extern std::vector<VkSemaphore> g_renderFinishedSemaphores;
extern std::vector<VkFence> g_inFlightFences;
extern uint32_t g_currentFrame;

// ===== Validation Layer =====
extern bool g_enableValidationLayers;
extern const std::vector<const char*> g_validationLayers;

// ===== 核心函数 =====
bool initVulkan();
void cleanupVulkan();