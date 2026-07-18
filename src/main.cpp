#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <chrono>

#include "sys/input_sample.h"
#include "sys/ui_manager.h"

#include "render/canvas.h"
#include "render/brush_renderer.h"

#include "engine/engine_state.h"
#include "engine/engine_init.h"
#include "engine/tool_switcher.h"
#include "engine/render_loop.h"
#include "engine/window_proc.h"

#include "vulkan/vk_core.h"
#include "vulkan/vk_canvas_tex.h"

#include "base/log.h"

// ===== 主函数 =====
int main(int argc, char* argv[]) {

    be::Log::init("logs", be::LogLevel::Debug);

    HINSTANCE hInstance = GetModuleHandle(nullptr);
    int nCmdShow = SW_SHOWDEFAULT;

    // 调试控制台
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);

    BE_LOG_INFO("BrushEngine initializing ...");

    if (!registerWindowClass(hInstance)) {
        BE_LOG_FATAL("Failed to register window class");
        return 1;
    }

    g_hwnd = CreateWindowEx(
        0,
        TEXT("BrushEngineClass"),
        TEXT("BrushEngine - Vulkan + ImGui"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_windowWidth + 16,
        g_windowHeight + 39,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!g_hwnd) {
        BE_LOG_FATAL("Failed to create window");
        return 1;
    }

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    // ===== 初始化 Vulkan =====
    if (!initVulkan()) {
        BE_LOG_ERROR("Vulkan initialization failed!");
        return 1;
    }

    // ===== 初始化引擎 =====
    if (!initEngine()) {
        BE_LOG_ERROR("Engine initialization failed!");
        return 1;
    }

    // ===== 初始化 ImGui =====
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
        BE_LOG_ERROR("ImGui initialization failed!");
        return 1;
    }

    // 创建画布纹理（必须在 ImGui Vulkan 后端初始化之后）
    if (!createCanvasTexture(g_canvas->width(), g_canvas->height())) {
        BE_LOG_ERROR("Canvas texture creation failed!");
        return 1;
    }
    g_uiManager->setCanvasTexture(g_canvasTextureId);
    BE_LOG_INFO("UI visible={}", g_uiManager->isVisible());

    // ===== 消息循环 =====
    MSG msg = {};
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - lastTime).count();
            lastTime = now;

            // 水彩等需要 FINALIZING 处理的笔刷
            if (g_currentToolType == ToolType::Brush && g_brushManager && g_brushManager->currentBrush() && g_brushManager->currentBrush()->isFinalizing()) {
                RenderContext ctx;
                ctx.deltaTime = dt;
                ctx.canvasScale = 1.0f;
                ctx.canvasOffset = Vec2(0, 0);
                g_brushManager->currentBrush()->render(*g_canvas, ctx);
            }
            BE_LOG_DEBUG(">>> entering renderFrame");
            renderFrame();
            BE_LOG_DEBUG("<<< leaving renderFrame");
            Sleep(16);
        }
    }

    // ===== 清理 =====
    vkDeviceWaitIdle(g_device);

    g_uiManager->shutdown();
    shutdownEngine();
    cleanupVulkan();

    BE_LOG_INFO("BrushEngine shutting down...");
    FreeConsole();

    return static_cast<int>(msg.wParam);
}