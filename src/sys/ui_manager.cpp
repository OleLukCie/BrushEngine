#define NOMINMAX
#include "ui_manager.h"
#include <algorithm>
#include <cmath>

// ============================================
// 构造函数 / 析构函数
// ============================================
UIManager::UIManager() {
    panels_.push_back({ "Tools",      true });
    panels_.push_back({ "Brush",      true });
    panels_.push_back({ "Layers",     true });
    panels_.push_back({ "Color",      true });
    panels_.push_back({ "History",    true });
    panels_.push_back({ "Properties", true });
}

UIManager::~UIManager() {
    if (initialized_) {
        shutdown();
    }
}

bool UIManager::init(
    HWND hwnd,
    VkInstance instance,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    uint32_t queueFamily,
    VkQueue queue,
    VkRenderPass renderPass,
    uint32_t minImageCount,
    uint32_t imageCount
) {
    device_ = device;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Docking Branch 核心配置
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // ===== 修复：Win32 后端没有实现 Vulkan Surface 回调，手动设置 =====
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_CreateVkSurface = [](ImGuiViewport* vp, ImU64 vk_inst,
        const void* vk_allocators,
        ImU64* out_vk_surface) -> int {
            HWND viewport_hwnd = (HWND)vp->PlatformHandleRaw;
            if (!viewport_hwnd) viewport_hwnd = (HWND)vp->PlatformHandle;

            VkWin32SurfaceCreateInfoKHR createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            createInfo.hwnd = viewport_hwnd;
            createInfo.hinstance = GetModuleHandle(nullptr);

            VkSurfaceKHR surface;
            VkResult err = vkCreateWin32SurfaceKHR((VkInstance)vk_inst, &createInfo,
                (const VkAllocationCallbacks*)vk_allocators, &surface);
            if (err != VK_SUCCESS) return 0;

            *out_vk_surface = (ImU64)surface;
            return 1;
        };

    setupImGuiStyle();

    ImGui_ImplWin32_Init(hwnd);

    createDescriptorPool(device);

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.ApiVersion = VK_API_VERSION_1_2;
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = physicalDevice;
    initInfo.Device = device;
    initInfo.QueueFamily = queueFamily;
    initInfo.Queue = queue;
    initInfo.DescriptorPool = descriptorPool_;
    initInfo.MinImageCount = minImageCount;
    initInfo.ImageCount = imageCount;

    // 新版 docking branch: RenderPass 和 MSAASamples 在 PipelineInfoMain 里
    initInfo.PipelineInfoMain.RenderPass = renderPass;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);

    initialized_ = true;
    return true;
}

void UIManager::createDescriptorPool(VkDevice device) {
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,   1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,   1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,         1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
    poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool_);
}

void UIManager::newFrame() {
    if (!visible_ || !initialized_) return;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // ===== 1. 全屏 DockSpace Host =====
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags hostFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpaceHost", nullptr, hostFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspaceId = ImGui::GetID("MainDockSpace");

    // ===== 关键：用 DockBuilder 预设 PS 风格布局 =====
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;

        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

        // ========== PS 标准布局结构 ==========
        // 
        //  ┌──────────┬──────────────────────┬─────────────┐
        //  │  工具栏   │    选项栏 (Options)   │             │
        //  │ (窄条)   ├──────────────────────┤  右侧面板组   │
        //  │          │                      │  - 颜色       │
        //  │          │     画布区域          │  - 图层       │
        //  │          │     (中央节点)        │  - 历史       │
        //  │          │                      │  - 属性       │
        //  ├──────────┴──────────────────────┴─────────────┤
        //  │  状态栏 (Status Bar)                           │
        //  └───────────────────────────────────────────────┘

        ImGuiID dockLeft, dockRight, dockCenter, dockOptions, dockStatus;

        // 第一步：从左分裂出工具栏（窄条，约 4.5% 宽度）
        ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.045f, &dockLeft, &dockCenter);

        // 第二步：从右侧分裂出面板组（约 22% 宽度）
        ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Right, 0.22f, &dockRight, &dockCenter);

        // 第三步：从中央区域顶部分裂出选项栏（约 4.5% 高度）
        ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Up, 0.045f, &dockOptions, &dockCenter);

        // 第四步：从中央区域底部分裂出状态栏（约 3.5% 高度）
        ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Down, 0.035f, &dockStatus, &dockCenter);

        // ===== 锁定各个节点，防止用户拖拽调整 =====
        ImGuiDockNode* leftNode = ImGui::DockBuilderGetNode(dockLeft);
        ImGuiDockNode* rightNode = ImGui::DockBuilderGetNode(dockRight);
        ImGuiDockNode* optionsNode = ImGui::DockBuilderGetNode(dockOptions);
        ImGuiDockNode* statusNode = ImGui::DockBuilderGetNode(dockStatus);
        ImGuiDockNode* centerNode = ImGui::DockBuilderGetNode(dockCenter);

        if (leftNode) {
            leftNode->LocalFlags |= ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoDockingSplit | ImGuiDockNodeFlags_NoUndocking;
        }
        if (rightNode) {
            rightNode->LocalFlags |= ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoDockingSplit | ImGuiDockNodeFlags_NoUndocking;
        }
        if (optionsNode) {
            optionsNode->LocalFlags |= ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoDockingSplit | ImGuiDockNodeFlags_NoUndocking;
        }
        if (statusNode) {
            statusNode->LocalFlags |= ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoDockingSplit | ImGuiDockNodeFlags_NoUndocking;
        }
        if (centerNode) {
            centerNode->LocalFlags |= ImGuiDockNodeFlags_NoDockingOverCentralNode | ImGuiDockNodeFlags_NoDockingSplit;
        }

        // 停靠窗口到对应区域
        ImGui::DockBuilderDockWindow("Tools", dockLeft);
        ImGui::DockBuilderDockWindow("OptionsBar", dockOptions);
        ImGui::DockBuilderDockWindow("Color", dockRight);
        ImGui::DockBuilderDockWindow("Layers", dockRight);
        ImGui::DockBuilderDockWindow("History", dockRight);
        ImGui::DockBuilderDockWindow("Properties", dockRight);
        ImGui::DockBuilderDockWindow("StatusBar", dockStatus);
        ImGui::DockBuilderDockWindow("Canvas", dockCenter);

        ImGui::DockBuilderFinish(dockspaceId);
    }

    ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f),
        ImGuiDockNodeFlags_NoDockingInCentralNode);
    ImGui::PopStyleColor();

    // 菜单栏
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {}
            if (ImGui::MenuItem("Open", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Preferences")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Tools Panel", nullptr, &findPanel("Tools")->open);
            ImGui::MenuItem("Brush Panel", nullptr, &findPanel("Brush")->open);
            ImGui::MenuItem("Layers Panel", nullptr, &findPanel("Layers")->open);
            ImGui::MenuItem("Color Panel", nullptr, &findPanel("Color")->open);
            ImGui::MenuItem("History Panel", nullptr, &findPanel("History")->open);
            ImGui::MenuItem("Properties Panel", nullptr, &findPanel("Properties")->open);
            ImGui::Separator();
            if (ImGui::MenuItem("Toggle UI", "H", visible_)) {
                setVisible(!visible_);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End(); // DockSpaceHost

    // ===== 2. 绘制各个面板 =====
    if (isPanelVisible("Tools"))      drawToolPanel();
    if (isPanelVisible("Brush"))      drawBrushPanel();
    if (isPanelVisible("Layers"))     drawLayerPanel();
    if (isPanelVisible("Color"))      drawColorPanel();
    if (isPanelVisible("History"))    drawHistoryPanel();
    if (isPanelVisible("Properties")) drawPropertiesPanel();

    // ===== 3. 选项栏 =====
    drawOptionsBar();

    // ===== 4. 状态栏 =====
    drawStatusBar();

    // ===== 5. 画布窗口 =====
    drawCanvasWindow();

    // ===== 6. 视口覆盖信息 =====
    drawViewportOverlay();
}

void UIManager::drawCanvasWindow() {
    if (!canvasTextureId_ || !canvas_) return;

    ImGuiWindowFlags canvasFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("Canvas", nullptr, canvasFlags)) {
        int canvasW = canvas_->width();
        int canvasH = canvas_->height();
        ImVec2 avail = ImGui::GetContentRegionAvail();

        float scaleX = avail.x / static_cast<float>(canvasW);
        float scaleY = avail.y / static_cast<float>(canvasH);
        float scale = std::min(scaleX, scaleY);
        scale = std::clamp(scale, 0.1f, 10.0f);
        canvasZoom_ = scale;

        ImVec2 imageSize(canvasW * scale, canvasH * scale);
        float offsetX = std::max(0.0f, (avail.x - imageSize.x) * 0.5f);
        float offsetY = std::max(0.0f, (avail.y - imageSize.y) * 0.5f);

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
        ImGui::Image(canvasTextureId_, imageSize);
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

void UIManager::render(VkCommandBuffer commandBuffer) {
    if (!visible_ || !initialized_) return;

    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

// ============================================
// 清理
// ============================================
void UIManager::shutdown() {
    if (!initialized_) return;

    // 1. 先 shutdown ImGui（这会释放内部的所有 descriptor set）
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // 2. 再销毁 descriptor pool（现在 pool 里的 set 都已经被释放了）
    if (device_ != VK_NULL_HANDLE && descriptorPool_ != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
        descriptorPool_ = VK_NULL_HANDLE;
    }

    device_ = VK_NULL_HANDLE;
    initialized_ = false;
}

void UIManager::onResize(uint32_t width, uint32_t height) {
    (void)width;
    (void)height;
}

// ============================================
// 面板显隐控制
// ============================================
UIManager::PanelState* UIManager::findPanel(const std::string& name) {
    for (auto& p : panels_) {
        if (p.name == name) return &p;
    }
    return nullptr;
}

void UIManager::togglePanel(const std::string& name) {
    if (auto* p = findPanel(name)) {
        p->open = !p->open;
    }
}

bool UIManager::isPanelVisible(const std::string& name) const {
    for (const auto& p : panels_) {
        if (p.name == name) return p.open;
    }
    return false;
}

bool UIManager::onKeyDown(int key) {
    if (key == 'H') {
        setVisible(!visible_);
        return true;
    }
    return false;
}

bool UIManager::onMouseWheel(float delta) {
    if (!visible_ || !brushes_) return false;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return false;

    auto* settings = brushes_->currentBrushSettings();
    if (settings) {
        float step = (settings->maxSize - settings->minSize) * 0.05f;
        settings->baseSize = std::clamp(
            settings->baseSize + delta * step,
            settings->minSize,
            settings->maxSize
        );
        return true;
    }
    return false;
}

// ============================================
// 样式设置
// ============================================
void UIManager::setupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.18f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.28f, 0.28f, 0.28f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.52f, 0.96f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.52f, 0.96f, 0.50f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.52f, 0.96f, 0.67f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.25f, 0.52f, 0.96f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.25f, 0.52f, 0.96f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.25f, 0.52f, 0.96f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.52f, 0.96f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
}

// ============================================
// 各面板绘制
// ============================================

void UIManager::drawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {}
            if (ImGui::MenuItem("Open", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Preferences")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Tools Panel", nullptr, &findPanel("Tools")->open);
            ImGui::MenuItem("Brush Panel", nullptr, &findPanel("Brush")->open);
            ImGui::MenuItem("Layers Panel", nullptr, &findPanel("Layers")->open);
            ImGui::MenuItem("Color Panel", nullptr, &findPanel("Color")->open);
            ImGui::MenuItem("History Panel", nullptr, &findPanel("History")->open);
            ImGui::MenuItem("Properties Panel", nullptr, &findPanel("Properties")->open);
            ImGui::Separator();
            if (ImGui::MenuItem("Toggle UI", "H", visible_)) {
                setVisible(!visible_);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

// ===== 选项栏（Options Bar） =====
void UIManager::drawOptionsBar() {
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.16f, 0.16f, 0.16f, 1.0f));

    if (ImGui::Begin("OptionsBar", nullptr, flags)) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Brush Tool");
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(20, 0));
        ImGui::SameLine();

        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        if (brushes_) {
            auto* settings = brushes_->currentBrushSettings();
            if (settings) {
                ImGui::Text("Size:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(60);
                ImGui::DragFloat("##opt_size", &settings->baseSize, 0.5f, settings->minSize, settings->maxSize, "%.0f");
                ImGui::SameLine();

                ImGui::Text("Opacity:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(60);
                ImGui::DragFloat("##opt_opacity", &settings->opacity, 0.01f, 0.0f, 1.0f, "%.0f%%");
                ImGui::SameLine();

                ImGui::Text("Flow:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(60);
                static float flow = 100.0f;
                ImGui::DragFloat("##opt_flow", &flow, 1.0f, 0.0f, 100.0f, "%.0f%%");
                ImGui::SameLine();

                ImGui::Text("Hardness:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(60);
                static float hardness = 100.0f;
                ImGui::DragFloat("##opt_hardness", &hardness, 1.0f, 0.0f, 100.0f, "%.0f%%");
            }
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 180);
        ImGui::Text("Mode:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        const char* modes[] = { "Normal", "Dissolve", "Behind", "Clear" };
        static int modeIdx = 0;
        ImGui::Combo("##blend_mode", &modeIdx, modes, IM_ARRAYSIZE(modes));
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

// ===== 状态栏（Status Bar） =====
void UIManager::drawStatusBar() {
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.14f, 0.14f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.70f, 1.0f));

    if (ImGui::Begin("StatusBar", nullptr, flags)) {
        if (canvas_) {
            ImGui::Text("%dx%d px", canvas_->width(), canvas_->height());
        }
        else {
            ImGui::Text("No document");
        }

        ImGui::SameLine(ImGui::GetWindowWidth() * 0.35f);
        if (brushes_) {
            ImGui::Text("%s | Click and drag to paint", brushes_->activeBrushName().c_str());
        }
        else {
            ImGui::Text("Ready");
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 140);
        ImGui::Text("Zoom: %.0f%%", canvasZoom_ * 100.0f);

        ImGui::SameLine(ImGui::GetWindowWidth() - 70);
        if (ImGui::Button("-", ImVec2(18, 16))) {
            canvasZoom_ = std::max(0.1f, canvasZoom_ - 0.1f);
        }
        ImGui::SameLine();
        if (ImGui::Button("+", ImVec2(18, 16))) {
            canvasZoom_ = std::min(10.0f, canvasZoom_ + 0.1f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Fit", ImVec2(28, 16))) {
            canvasZoom_ = 1.0f;
        }
    }
    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
}

void UIManager::drawToolPanel() {
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));

    if (ImGui::Begin("Tools", &findPanel("Tools")->open, flags)) {
        struct ToolDef { const char* icon; const char* name; const char* shortcut; };
        ToolDef tools[] = {
            { "B", "Brush Tool",          "B" },
            { "E", "Eraser Tool",         "E" },
            { "I", "Eyedropper Tool",     "I" },
            { "Z", "Zoom Tool",           "Z" },
            { "M", "Move Tool",           "V" },
            { "L", "Lasso Tool",          "L" },
            { "P", "Polygonal Lasso",     "Shift+L" },
            { "W", "Magic Wand",          "W" },
            { "R", "Rectangular Marquee", "M" },
            { "O", "Elliptical Marquee",  "Shift+M" },
            { "C", "Crop Tool",           "C" },
            { "T", "Type Tool",           "T" },
            { "U", "Rotate View Tool",    "R" },
            { "S", "Clone Stamp",         "S" },
            { "H", "Healing Brush",       "J" },
            { "G", "Gradient Tool",       "G" },
        };

        int cols = 2;
        for (int i = 0; i < IM_ARRAYSIZE(tools); ++i) {
            if (i > 0 && i % cols != 0) ImGui::SameLine();

            bool active = (activeToolIndex_ == i);
            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.52f, 0.96f, 0.60f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.52f, 0.96f, 0.80f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.52f, 0.96f, 1.00f));
            }

            if (ImGui::Button(tools[i].icon, ImVec2(32, 32))) {
                activeToolIndex_ = i;
            }

            if (active) {
                ImGui::PopStyleColor(3);
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s (%s)", tools[i].name, tools[i].shortcut);
            }
        }
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

void UIManager::drawBrushPanel() {
    if (!brushes_) {
        ImGui::Begin("Brush", &findPanel("Brush")->open);
        ImGui::TextDisabled("No brush manager");
        ImGui::End();
        return;
    }

    auto* settings = brushes_->currentBrushSettings();
    ImGui::Begin("Brush", &findPanel("Brush")->open);

    if (settings) {
        ImGui::Text("Size");
        ImGui::SliderFloat("##Size", &settings->baseSize, settings->minSize, settings->maxSize, "%.1f");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Mouse wheel to adjust");
        }

        ImGui::Text("Opacity");
        ImGui::SliderFloat("##Opacity", &settings->opacity, 0.0f, 1.0f, "%.2f");

        ImGui::Separator();

        ImGui::Text("Presets");
        auto brushList = brushes_->availableBrushes();
        for (const auto& name : brushList) {
            bool active = (name == brushes_->activeBrushName());
            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Header));
            }
            if (ImGui::Button(name.c_str(), ImVec2(-1, 0))) {
                brushes_->setActiveBrush(name);
            }
            if (active) {
                ImGui::PopStyleColor();
            }
        }
    }
    else {
        ImGui::TextDisabled("No active brush settings");
    }

    ImGui::End();
}

void UIManager::drawLayerPanel() {
    if (!canvas_) {
        ImGui::Begin("Layers", &findPanel("Layers")->open);
        ImGui::TextDisabled("No canvas");
        ImGui::End();
        return;
    }

    ImGui::Begin("Layers", &findPanel("Layers")->open);

    // ===== 混合模式和不透明度 =====
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

    const char* blendModes[] = {
        "Normal", "Dissolve", "Behind", "Clear",
        "Darken", "Multiply", "Color Burn", "Linear Burn",
        "Lighten", "Screen", "Color Dodge", "Linear Dodge",
        "Overlay", "Soft Light", "Hard Light", "Vivid Light",
        "Difference", "Exclusion", "Subtract", "Divide",
        "Hue", "Saturation", "Color", "Luminosity"
    };
    static int blendMode = 0;
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("##blend_mode", &blendMode, blendModes, IM_ARRAYSIZE(blendModes));

    static float layerOpacity = 100.0f;
    ImGui::SetNextItemWidth(-40);
    ImGui::SliderFloat("##layer_opacity", &layerOpacity, 0.0f, 100.0f, "%.0f%%");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(36);
    ImGui::DragFloat("##layer_opacity_val", &layerOpacity, 1.0f, 0.0f, 100.0f, "%.0f");

    ImGui::PopStyleVar();
    ImGui::Separator();

    // ===== 图层操作按钮 =====
    if (ImGui::Button("+", ImVec2(28, 22))) {
        canvas_->addLayer("New Layer");
    }
    ImGui::SameLine();
    if (ImGui::Button("-", ImVec2(28, 22))) {
        if (canvas_->layerCount() > 1) {
            canvas_->removeLayer(canvas_->activeLayerIndex());
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Up", ImVec2(32, 22))) {
        size_t idx = canvas_->activeLayerIndex();
        if (idx < canvas_->layerCount() - 1) {
            canvas_->moveLayer(idx, idx + 1);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Down", ImVec2(38, 22))) {
        size_t idx = canvas_->activeLayerIndex();
        if (idx > 0) {
            canvas_->moveLayer(idx, idx - 1);
        }
    }

    ImGui::Separator();

    // ===== 图层列表 =====
    for (int i = static_cast<int>(canvas_->layerCount()) - 1; i >= 0; --i) {
        auto& layer = canvas_->layer(i);
        bool active = (i == static_cast<int>(canvas_->activeLayerIndex()));

        ImGui::PushID(i);

        bool visible = layer.visible();
        if (ImGui::Checkbox("##vis", &visible)) {
            layer.setVisible(visible);
        }
        ImGui::SameLine();

        static bool locked = false;
        if (ImGui::Button(locked ? "L" : "U", ImVec2(18, 18))) {
            locked = !locked;
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Lock/Unlock layer");
        ImGui::SameLine();

        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        }
        if (ImGui::Button(layer.name().c_str(), ImVec2(-1, 0))) {
            canvas_->setActiveLayer(i);
        }
        if (active) {
            ImGui::PopStyleColor();
        }

        ImGui::PopID();
    }

    ImGui::End();
}

void UIManager::drawColorPanel() {
    ImGui::Begin("Color", &findPanel("Color")->open);

    // ===== PS 风格：前景/背景色方块 =====
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float btnSize = std::min(avail.x * 0.35f, 50.0f);

    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float offset = btnSize * 0.35f;

    // 背景色方块（左下）
    ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y + offset));
    ImGui::ColorButton("##bg", ImVec4(bgColor_[0], bgColor_[1], bgColor_[2], bgColor_[3]),
        ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder, ImVec2(btnSize, btnSize));
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Background Color");

    // 前景色方块（右上，重叠）
    ImGui::SetCursorScreenPos(ImVec2(cursor.x + offset, cursor.y));
    ImGui::ColorButton("##fg", ImVec4(fgColor_[0], fgColor_[1], fgColor_[2], fgColor_[3]),
        ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder, ImVec2(btnSize, btnSize));
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Foreground Color");

    // 交换和默认按钮
    ImGui::SetCursorScreenPos(ImVec2(cursor.x + btnSize + offset + 8, cursor.y + btnSize * 0.25f));
    if (ImGui::Button("<->", ImVec2(24, 24))) {
        std::swap(fgColor_[0], bgColor_[0]);
        std::swap(fgColor_[1], bgColor_[1]);
        std::swap(fgColor_[2], bgColor_[2]);
        std::swap(fgColor_[3], bgColor_[3]);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Swap Colors (X)");

    ImGui::SameLine();
    if (ImGui::Button("D", ImVec2(24, 24))) {
        fgColor_[0] = 0.0f; fgColor_[1] = 0.0f; fgColor_[2] = 0.0f; fgColor_[3] = 1.0f;
        bgColor_[0] = 1.0f; bgColor_[1] = 1.0f; bgColor_[2] = 1.0f; bgColor_[3] = 1.0f;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Default Colors (D)");

    ImGui::Dummy(ImVec2(0, btnSize + 8));

    ImGui::Separator();

    // ===== 色轮 + RGB 滑块 =====
    ImGui::ColorPicker4("##picker", fgColor_,
        ImGuiColorEditFlags_DisplayRGB |
        ImGuiColorEditFlags_PickerHueWheel |
        ImGuiColorEditFlags_NoSidePreview |
        ImGuiColorEditFlags_NoSmallPreview);

    ImGui::Separator();

    // ===== 预设色板 =====
    ImGui::Text("Swatches");
    const ImVec4 swatches[] = {
        ImVec4(0,0,0,1), ImVec4(0.5f,0.5f,0.5f,1), ImVec4(1,1,1,1),
        ImVec4(1,0,0,1), ImVec4(0,1,0,1), ImVec4(0,0,1,1),
        ImVec4(1,1,0,1), ImVec4(0,1,1,1), ImVec4(1,0,1,1),
        ImVec4(1,0.5f,0,1), ImVec4(0.5f,0,1,1), ImVec4(0,0.5f,0.5f,1),
    };
    for (int i = 0; i < IM_ARRAYSIZE(swatches); ++i) {
        if (i > 0 && i % 6 != 0) ImGui::SameLine();
        ImGui::PushID(i);
        if (ImGui::ColorButton("##swatch", swatches[i], ImGuiColorEditFlags_NoTooltip, ImVec2(22, 22))) {
            fgColor_[0] = swatches[i].x;
            fgColor_[1] = swatches[i].y;
            fgColor_[2] = swatches[i].z;
            fgColor_[3] = swatches[i].w;
        }
        ImGui::PopID();
    }

    ImGui::End();
}

void UIManager::drawHistoryPanel() {
    ImGui::Begin("History", &findPanel("History")->open);

    const char* historyItems[] = {
        "Open", "Brush Stroke", "Brush Stroke", "Eraser",
        "Brush Stroke", "Move Layer", "Add Layer", "Delete Layer"
    };
    static int historyIndex = 5;

    ImGui::Text("History");
    ImGui::Separator();

    for (int i = 0; i < IM_ARRAYSIZE(historyItems); ++i) {
        bool isCurrent = (i == historyIndex);
        if (isCurrent) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.25f, 0.52f, 0.96f, 1.0f));
        }
        ImGui::Selectable(historyItems[i], isCurrent);
        if (isCurrent) {
            ImGui::PopStyleColor();
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Clear History", ImVec2(-1, 0))) {
        // 清除历史
    }

    ImGui::End();
}

void UIManager::drawPropertiesPanel() {
    ImGui::Begin("Properties", &findPanel("Properties")->open);

    if (canvas_) {
        ImGui::Text("Document");
        ImGui::Separator();

        static int width = 1920, height = 1080;
        ImGui::Text("Width:");  ImGui::SameLine(70);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputInt("##doc_w", &width);

        ImGui::Text("Height:"); ImGui::SameLine(70);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputInt("##doc_h", &height);

        ImGui::Text("Resolution:"); ImGui::SameLine(70);
        ImGui::SetNextItemWidth(-1);
        static float dpi = 72.0f;
        ImGui::InputFloat("##doc_dpi", &dpi, 0, 0, "%.0f PPI");

        ImGui::Separator();
        ImGui::Text("Color Mode: RGB");
        ImGui::Text("Bit Depth: 8 bits/channel");
    }
    else {
        ImGui::TextDisabled("No document properties available");
    }

    ImGui::End();
}

void UIManager::drawViewportOverlay() {
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoBackground;

    ImGui::SetNextWindowPos(ImVec2(10, 60), ImGuiCond_Always, ImVec2(0, 0));

    if (ImGui::Begin("ViewportOverlay", nullptr, flags)) {
        if (canvas_) {
            ImGui::Text("Canvas: %dx%d", canvas_->width(), canvas_->height());
        }
        if (brushes_) {
            ImGui::Text("Tool: %s", brushes_->activeBrushName().c_str());
            auto* s = brushes_->currentBrushSettings();
            if (s) {
                ImGui::Text("Size: %.1f | Opacity: %.2f", s->baseSize, s->opacity);
            }
        }
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    }
    ImGui::End();
}