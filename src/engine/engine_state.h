#pragma once
#include <windows.h>
#include <windowsx.h>
#include <memory>
#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include <imgui.h>

#include "sys/tool_manager.h"
#include "tool/tool_base.h"
#include "sys/ui_manager.h"

// 前向声明
class Canvas;
class BrushManager;
class ShortcutManager;

// ===== 窗口状态 =====
extern HWND g_hwnd;
extern int g_windowWidth;
extern int g_windowHeight;
extern bool g_framebufferResized;

// ===== 引擎核心对象 =====
extern std::unique_ptr<Canvas> g_canvas;
extern std::unique_ptr<BrushManager> g_brushManager;
extern std::unique_ptr<UIManager> g_uiManager;
extern std::unique_ptr<ShortcutManager> g_shortcutManager;

// ===== 当前工具 =====
extern std::unique_ptr<Tool> g_currentTool;
extern ToolType g_currentToolType;

// ===== 输入状态 =====
extern bool g_mouseDown;
extern float g_lastMouseX;
extern float g_lastMouseY;

// ===== Vulkan 全局对象 =====
extern VkInstance g_instance;
extern VkPhysicalDevice g_physicalDevice;
extern VkDevice g_device;
extern VkQueue g_graphicsQueue;
extern VkQueue g_presentQueue;
extern VkSurfaceKHR g_surface;
extern VkSwapchainKHR g_swapchain;
extern std::vector<VkImage> g_swapchainImages;
extern std::vector<VkImageView> g_swapchainImageViews;
extern VkFormat g_swapchainFormat;
extern VkExtent2D g_swapchainExtent;
extern VkRenderPass g_renderPass;
extern std::vector<VkFramebuffer> g_framebuffers;
extern VkCommandPool g_commandPool;
extern std::vector<VkCommandBuffer> g_commandBuffers;
extern VkDescriptorPool g_imguiDescriptorPool;

// 同步对象
extern uint32_t g_imageCount;
extern uint32_t g_maxFramesInFlight;
extern uint32_t g_currentFrame;
extern std::vector<VkSemaphore> g_imageAvailableSemaphores;
extern std::vector<VkSemaphore> g_renderFinishedSemaphores;
extern std::vector<VkFence> g_inFlightFences;

extern uint32_t g_graphicsQueueFamily;
extern uint32_t g_presentQueueFamily;

// ===== 画布纹理 =====
extern VkImage g_canvasImage;
extern VkDeviceMemory g_canvasImageMemory;
extern VkImageView g_canvasImageView;
extern VkSampler g_canvasSampler;
extern VkBuffer g_canvasStagingBuffer;
extern VkDeviceMemory g_canvasStagingBufferMemory;
extern VkDeviceSize g_canvasStagingSize;
extern ImTextureID g_canvasTextureId;
extern bool g_canvasFirstUpload;

extern std::vector<uint32_t> g_imagesInFlight;