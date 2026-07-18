#define NOMINMAX
#include "engine_init.h"
#include "engine_state.h"
#include "tool_switcher.h"

#include "render/canvas.h"
#include "sys/brush_circle.h"
#include "sys/brush_marker.h"
#include "sys/brush_pencil.h"
#include "sys/brush_watercolor.h"
#include "sys/tool_manager.h"
#include "sys/ui_manager.h"
#include "sys/shortcut.h"

#include "tool/tool_move.h"
#include "tool/tool_eraser.h"
#include "tool/tool_eyedropper.h"
#include "tool/tool_zoom.h"
#include "tool/tool_rotate_canvas.h"
#include "tool/tool_rect_select.h"
#include "tool/tool_ellipse_select.h"
#include "tool/tool_lasso.h"
#include "tool/tool_poly_lasso.h"
#include "tool/tool_magic_wand.h"
#include "tool/tool_crop.h"
#include "tool/tool_pen.h"
#include "tool/tool_text.h"

#include "base/log.h"

bool initEngine() {
    // 1. 先创建画布
    g_canvas = std::make_unique<Canvas>(g_windowWidth, g_windowHeight);
    g_canvas->addLayer("Background");
    g_canvas->addLayer("Drawing");
    g_canvas->layer(0).clear(Color(1, 1, 1, 1));

    // 2. 创建笔刷管理器
    g_brushManager = std::make_unique<BrushManager>();
    g_brushManager->registerBrush("Circle", []() { return std::make_unique<BrushCircle>(); });
    g_brushManager->registerBrush("Marker", []() { return std::make_unique<BrushMarker>(); });
    g_brushManager->registerBrush("Pencil", []() { return std::make_unique<BrushPencil>(); });
    g_brushManager->registerBrush("Watercolor", []() { return std::make_unique<BrushWatercolor>(); });
    g_brushManager->setActiveBrush("Circle");

    // 3. 创建 UI 管理器 (ImGui)
    g_uiManager = std::make_unique<UIManager>();
    g_uiManager->setCanvas(g_canvas.get());
    g_uiManager->setBrushManager(g_brushManager.get());

    // 4. 创建快捷键管理器
    g_shortcutManager = std::make_unique<ShortcutManager>();
    g_shortcutManager->initDefaults();

    // 注册快捷键处理器
    g_shortcutManager->registerHandler(ShortcutCommand::ToolBrush, [](ShortcutCommand) { setTool(ToolType::Brush); });
    g_shortcutManager->registerHandler(ShortcutCommand::ToolMove, [](ShortcutCommand) { setTool(ToolType::Move); });
    g_shortcutManager->registerHandler(ShortcutCommand::ToolEraser, [](ShortcutCommand) { setTool(ToolType::Eraser); });
    g_shortcutManager->registerHandler(ShortcutCommand::ToolEyedropper, [](ShortcutCommand) { setTool(ToolType::Eyedropper); });
    g_shortcutManager->registerHandler(ShortcutCommand::ToolZoom, [](ShortcutCommand) { setTool(ToolType::Zoom); });
    g_shortcutManager->registerHandler(ShortcutCommand::ToolRotateCanvas, [](ShortcutCommand) { setTool(ToolType::RotateCanvas); });
    g_shortcutManager->registerHandler(ShortcutCommand::ToolRectSelect, [](ShortcutCommand) { setTool(ToolType::RectSelect); });
    g_shortcutManager->registerHandler(ShortcutCommand::ToolLasso, [](ShortcutCommand) { setTool(ToolType::Lasso); });
    g_shortcutManager->registerHandler(ShortcutCommand::ToolMagicWand, [](ShortcutCommand) { setTool(ToolType::MagicWand); });
    g_shortcutManager->registerHandler(ShortcutCommand::ToolCrop, [](ShortcutCommand) { setTool(ToolType::Crop); });
    g_shortcutManager->registerHandler(ShortcutCommand::ToolPen, [](ShortcutCommand) { setTool(ToolType::Pen); });
    g_shortcutManager->registerHandler(ShortcutCommand::ToolText, [](ShortcutCommand) { setTool(ToolType::Text); });
    g_shortcutManager->registerHandler(ShortcutCommand::FileNew, [](ShortcutCommand) {
        if (g_canvas) {
            g_canvas->layer(1).clear(Color(0, 0, 0, 0));
        }
    });

    return true;
}

void shutdownEngine() {
    g_currentTool.reset();
    g_brushManager.reset();
    g_shortcutManager.reset();
    g_uiManager.reset();
    g_canvas.reset();
}