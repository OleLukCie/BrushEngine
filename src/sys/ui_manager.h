#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <algorithm>
#include <utility>

#include "../render/canvas.h"
#include "tool_manager.h"
#include <imgui.h>
#include "imgui_internal.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_vulkan.h>

class UIManager {
public:
    UIManager();
    ~UIManager();

    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    bool init(
        HWND hwnd,
        VkInstance instance,
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        uint32_t queueFamily,
        VkQueue queue,
        VkRenderPass renderPass,
        uint32_t minImageCount,
        uint32_t imageCount
    );

    void newFrame();
    void render(VkCommandBuffer commandBuffer);
    void onResize(uint32_t width, uint32_t height);
    void shutdown();

    void setCanvas(Canvas* canvas) { canvas_ = canvas; }
    void setBrushManager(BrushManager* brushes) { brushes_ = brushes; }
    void setCanvasTexture(ImTextureID textureId) { canvasTextureId_ = textureId; }

    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }

    void togglePanel(const std::string& name);
    bool isPanelVisible(const std::string& name) const;

    bool onMouseWheel(float delta);
    bool onKeyDown(int key);

    ImTextureID canvasTextureId_ = {};

private:
    Canvas* canvas_ = nullptr;
    BrushManager* brushes_ = nullptr;
    bool visible_ = true;

    VkDevice device_ = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;
    bool initialized_ = false;

    struct PanelState {
        std::string name;
        bool open = true;
    };
    std::vector<PanelState> panels_;
    PanelState* findPanel(const std::string& name);

    // ===== PS 布局新增成员 =====
    int activeToolIndex_ = 0;           // 当前选中工具索引
    float canvasZoom_ = 1.0f;           // 画布缩放比例
    float fgColor_[4] = { 0.0f, 0.0f, 0.0f, 1.0f };  // 前景色 (RGBA)
    float bgColor_[4] = { 1.0f, 1.0f, 1.0f, 1.0f };  // 背景色 (RGBA)

    // ===== 核心面板绘制 =====
    void drawToolPanel();           // 左侧窄条工具栏
    void drawBrushPanel();          // 画笔设置面板
    void drawLayerPanel();          // 图层面板（含混合模式/不透明度）
    void drawColorPanel();          // 颜色面板（PS 风格）
    void drawHistoryPanel();        // 历史记录面板
    void drawPropertiesPanel();     // 属性面板
    void drawMenuBar();             // 菜单栏（备用）
    void drawViewportOverlay();     // 左上角 HUD 覆盖层

    // ===== PS 布局新增面板 =====
    void drawOptionsBar();          // 顶部选项栏（工具参数）
    void drawStatusBar();           // 底部状态栏（缩放/文档信息）
    void drawCanvasWindow();        // 画布窗口（强制占满中央）

    float color_[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    void setupImGuiStyle();
    void createDescriptorPool(VkDevice device);
};