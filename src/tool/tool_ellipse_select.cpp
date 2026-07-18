#include "tool_ellipse_select.h"
#include "../base/base_math.h"

void ToolEllipseSelect::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    selecting_ = true;
    dragStart_ = Vec2(sample.x, sample.y);
    dragCurrent_ = dragStart_;
}

void ToolEllipseSelect::onMouseMove(const InputSample& sample, Canvas& canvas) {
    if (!selecting_) return;
    dragCurrent_ = Vec2(sample.x, sample.y);
}

void ToolEllipseSelect::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0 || !selecting_) return;
    selecting_ = false;
    
    center_ = Vec2((dragStart_.x + dragCurrent_.x) * 0.5f, (dragStart_.y + dragCurrent_.y) * 0.5f);
    radiusX_ = std::abs(dragCurrent_.x - dragStart_.x) * 0.5f;
    radiusY_ = std::abs(dragCurrent_.y - dragStart_.y) * 0.5f;
    
    if (radiusX_ > 2.0f && radiusY_ > 2.0f) {
        hasSelection_ = true;
    }
}

void ToolEllipseSelect::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    if (!hasSelection_ && !selecting_) return;
    
    Vec2 c = selecting_ ? Vec2((dragStart_.x + dragCurrent_.x) * 0.5f, (dragStart_.y + dragCurrent_.y) * 0.5f) : center_;
    float rx = selecting_ ? std::abs(dragCurrent_.x - dragStart_.x) * 0.5f : radiusX_;
    float ry = selecting_ ? std::abs(dragCurrent_.y - dragStart_.y) * 0.5f : radiusY_;
    
    Color col = selecting_ ? Color(0.3f, 0.6f, 1.0f, 0.4f) : Color(1, 1, 1, 0.8f);
    
    // 中点椭圆算法绘制
    int cx = static_cast<int>(c.x);
    int cy = static_cast<int>(c.y);
    int a = static_cast<int>(rx);
    int b = static_cast<int>(ry);
    
    if (a <= 0 || b <= 0) return;
    
    // 简化：参数方程采样
    int segments = std::max(a, b) * 4;
    for (int i = 0; i < segments; ++i) {
        float t = 2.0f * PI * i / segments;
        int x = cx + static_cast<int>(a * std::cos(t));
        int y = cy + static_cast<int>(b * std::sin(t));
        overlay.setPixelSafe(x, y, col);
    }
}