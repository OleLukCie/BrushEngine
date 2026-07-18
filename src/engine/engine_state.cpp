#define NOMINMAX
#include "engine_state.h"

// ===== 窗口状态 =====
HWND g_hwnd = nullptr;
int g_windowWidth = 1280;
int g_windowHeight = 720;
bool g_framebufferResized = false;

// ===== 引擎核心对象 =====
std::unique_ptr<Canvas> g_canvas;
std::unique_ptr<BrushManager> g_brushManager;
std::unique_ptr<UIManager> g_uiManager;
std::unique_ptr<ShortcutManager> g_shortcutManager;

// ===== 当前工具 =====
std::unique_ptr<Tool> g_currentTool;
ToolType g_currentToolType = ToolType::Brush;

// ===== 输入状态 =====
bool g_mouseDown = false;
float g_lastMouseX = 0;
float g_lastMouseY = 0;

// ===== Vulkan 全局对象 =====
VkInstance g_instance = VK_NULL_HANDLE;
VkPhysicalDevice g_physicalDevice = VK_NULL_HANDLE;
VkDevice g_device = VK_NULL_HANDLE;
VkQueue g_graphicsQueue = VK_NULL_HANDLE;
VkQueue g_presentQueue = VK_NULL_HANDLE;
VkSurfaceKHR g_surface = VK_NULL_HANDLE;
VkSwapchainKHR g_swapchain = VK_NULL_HANDLE;
std::vector<VkImage> g_swapchainImages;
std::vector<VkImageView> g_swapchainImageViews;
VkFormat g_swapchainFormat;
VkExtent2D g_swapchainExtent;
VkRenderPass g_renderPass = VK_NULL_HANDLE;
std::vector<VkFramebuffer> g_framebuffers;
VkCommandPool g_commandPool = VK_NULL_HANDLE;
std::vector<VkCommandBuffer> g_commandBuffers;
VkDescriptorPool g_imguiDescriptorPool = VK_NULL_HANDLE;

// 同步对象
uint32_t g_imageCount = 0;
uint32_t g_maxFramesInFlight = 0;
uint32_t g_currentFrame = 0;
std::vector<VkSemaphore> g_imageAvailableSemaphores;
std::vector<VkSemaphore> g_renderFinishedSemaphores;
std::vector<VkFence> g_inFlightFences;

uint32_t g_graphicsQueueFamily = 0;
uint32_t g_presentQueueFamily = 0;

// ===== 画布纹理 =====
VkImage g_canvasImage = VK_NULL_HANDLE;
VkDeviceMemory g_canvasImageMemory = VK_NULL_HANDLE;
VkImageView g_canvasImageView = VK_NULL_HANDLE;
VkSampler g_canvasSampler = VK_NULL_HANDLE;
VkBuffer g_canvasStagingBuffer = VK_NULL_HANDLE;
VkDeviceMemory g_canvasStagingBufferMemory = VK_NULL_HANDLE;
VkDeviceSize g_canvasStagingSize = 0;
ImTextureID g_canvasTextureId = (ImTextureID)0;
bool g_canvasFirstUpload = true;
std::vector<uint32_t> g_imagesInFlight;