#define NOMINMAX
#include "tool_switcher.h"
#include "engine_state.h"

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

#include <iostream>

void setTool(ToolType type) {
    if (g_brushManager && g_brushManager->currentBrush() && g_brushManager->currentBrush()->isDrawing()) {
        g_brushManager->currentBrush()->onStrokeEnd();
    }

    g_currentToolType = type;

    switch (type) {
    case ToolType::Brush:
        g_currentTool.reset();
        std::cout << "Tool: Brush (" << g_brushManager->activeBrushName() << ")" << std::endl;
        break;
    case ToolType::Move:
        g_currentTool = std::make_unique<ToolMove>();
        std::cout << "Tool: Move" << std::endl;
        break;
    case ToolType::Eraser:
        g_currentTool = std::make_unique<ToolEraser>();
        std::cout << "Tool: Eraser" << std::endl;
        break;
    case ToolType::Eyedropper:
        g_currentTool = std::make_unique<ToolEyedropper>();
        std::cout << "Tool: Eyedropper" << std::endl;
        break;
    case ToolType::Zoom:
        g_currentTool = std::make_unique<ToolZoom>();
        std::cout << "Tool: Zoom" << std::endl;
        break;
    case ToolType::RotateCanvas:
        g_currentTool = std::make_unique<ToolRotateCanvas>();
        std::cout << "Tool: Rotate Canvas" << std::endl;
        break;
    case ToolType::RectSelect:
        g_currentTool = std::make_unique<ToolRectSelect>();
        std::cout << "Tool: Rect Select" << std::endl;
        break;
    case ToolType::EllipseSelect:
        g_currentTool = std::make_unique<ToolEllipseSelect>();
        std::cout << "Tool: Ellipse Select" << std::endl;
        break;
    case ToolType::Lasso:
        g_currentTool = std::make_unique<ToolLasso>();
        std::cout << "Tool: Lasso" << std::endl;
        break;
    case ToolType::PolyLasso:
        g_currentTool = std::make_unique<ToolPolyLasso>();
        std::cout << "Tool: Poly Lasso" << std::endl;
        break;
    case ToolType::MagicWand:
        g_currentTool = std::make_unique<ToolMagicWand>();
        std::cout << "Tool: Magic Wand" << std::endl;
        break;
    case ToolType::Crop:
        g_currentTool = std::make_unique<ToolCrop>();
        std::cout << "Tool: Crop" << std::endl;
        break;
    case ToolType::Pen:
        g_currentTool = std::make_unique<ToolPen>();
        std::cout << "Tool: Pen" << std::endl;
        break;
    case ToolType::Text:
        g_currentTool = std::make_unique<ToolText>();
        std::cout << "Tool: Text" << std::endl;
        break;
    default:
        break;
    }
}

void setBrush(const std::string& name) {
    if (!g_brushManager) return;
    g_brushManager->setActiveBrush(name);
    g_currentToolType = ToolType::Brush;
    g_currentTool.reset();
    std::cout << "Brush: " << name << std::endl;
}