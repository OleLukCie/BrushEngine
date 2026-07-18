#define NOMINMAX
#include "window_proc.h"
#include "engine_state.h"
#include "tool_switcher.h"
#include "sys/input_sample.h"

#include "sys/ui_manager.h"
#include "sys/shortcut.h"
#include "render/brush_renderer.h"
#include "render/canvas.h"
#include "tool/tool_base.h"

#include <imgui.h>
#include <imgui_impl_win32.h>

// ImGui Win32 回调（必须暴露给 ImGui）
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_CREATE:
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE: {
        g_windowWidth = LOWORD(lParam);
        g_windowHeight = HIWORD(lParam);
        g_framebufferResized = true;

        if (g_uiManager) {
            g_uiManager->onResize(g_windowWidth, g_windowHeight);
        }
        return 0;
    }

    case WM_MOUSEMOVE: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        g_lastMouseX = static_cast<float>(x);
        g_lastMouseY = static_cast<float>(y);

        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return 0;

        if (g_mouseDown) {
            if (g_currentToolType == ToolType::Brush && g_brushManager->currentBrush() && g_brushManager->currentBrush()->isDrawing()) {
                auto sample = makeInputSample(x, y);
                g_brushManager->currentBrush()->onStrokeMove(sample);
            }
            else if (g_currentTool && g_currentTool->isDragging()) {
                auto sample = makeInputSample(x, y);
                g_currentTool->onMouseMove(sample, *g_canvas);
            }
        }
        else if (g_currentTool && g_currentToolType != ToolType::Brush) {
            auto sample = makeInputSample(x, y);
            g_currentTool->onMouseMove(sample, *g_canvas);
        }
        return 0;
    }

    case WM_LBUTTONDOWN: {
        g_mouseDown = true;
        SetCapture(hwnd);

        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return 0;

        if (g_currentToolType == ToolType::Brush && g_brushManager->currentBrush()) {
            auto sample = makeInputSample(x, y);
            g_brushManager->currentBrush()->onStrokeBegin(sample);
        }
        else if (g_currentTool) {
            auto sample = makeInputSample(x, y);
            g_currentTool->onMouseDown(sample, *g_canvas, 0);
        }
        return 0;
    }

    case WM_LBUTTONUP: {
        g_mouseDown = false;
        ReleaseCapture();

        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return 0;

        if (g_currentToolType == ToolType::Brush && g_brushManager->currentBrush() && g_brushManager->currentBrush()->isDrawing()) {
            g_brushManager->currentBrush()->onStrokeEnd();
            RenderContext ctx;
            ctx.deltaTime = 0.016f;
            ctx.canvasScale = 1.0f;
            ctx.canvasOffset = Vec2(0, 0);
            g_brushManager->currentBrush()->render(*g_canvas, ctx);
        }
        else if (g_currentTool) {
            auto sample = makeInputSample(x, y);
            g_currentTool->onMouseUp(sample, *g_canvas, 0);
        }
        return 0;
    }

    case WM_RBUTTONDOWN: {
        return 0;
    }

    case WM_MOUSEWHEEL: {
        float delta = GET_WHEEL_DELTA_WPARAM(wParam) / 120.0f;

        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return 0;

        if (g_uiManager->onMouseWheel(delta)) {
            return 0;
        }

        if (g_currentTool) {
            g_currentTool->onMouseWheel(delta, *g_canvas);
        }
        return 0;
    }

    case WM_KEYDOWN: {
        bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        bool alt = (GetKeyState(VK_MENU) & 0x8000) != 0;

        if (wParam == 'H' && !ctrl && !shift && !alt) {
            g_uiManager->setVisible(!g_uiManager->isVisible());
            return 0;
        }

        auto key = ShortcutUtils::fromWinVK(static_cast<uint32_t>(wParam), ctrl, shift, alt);
        if (g_shortcutManager->onKeyEvent(key, true)) {
            return 0;
        }

        if (g_currentTool) {
            g_currentTool->onKeyDown(static_cast<int>(wParam), ctrl, shift, alt, *g_canvas);
        }
        return 0;
    }

    case WM_KEYUP: {
        if (g_currentTool) {
            g_currentTool->onKeyUp(static_cast<int>(wParam), *g_canvas);
        }
        return 0;
    }

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

bool registerWindowClass(HINSTANCE hInstance) {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("BrushEngineClass");
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    return RegisterClassEx(&wc) != 0;
}