#include "tool_rotate_canvas.h"
#include "../base/base_math.h"

void ToolRotateCanvas::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    
    rotating_ = true;
    center_ = Vec2(canvas.width() * 0.5f, canvas.height() * 0.5f);
    dragStart_ = Vec2(sample.x, sample.y);
    
    float dx = dragStart_.x - center_.x;
    float dy = dragStart_.y - center_.y;
    startAngle_ = std::atan2(dy, dx);
}

void ToolRotateCanvas::onMouseMove(const InputSample& sample, Canvas& canvas) {
    if (!rotating_) return;
    
    Vec2 current(sample.x, sample.y);
    float dx = current.x - center_.x;
    float dy = current.y - center_.y;
    float angle = std::atan2(dy, dx);
    
    currentAngle_ = angle - startAngle_;
    if (onRotate_) onRotate_(currentAngle_);
}

void ToolRotateCanvas::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    rotating_ = false;
}

void ToolRotateCanvas::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    if (!rotating_) return;
    
    // 绘制旋转中心点和角度指示
    int cx = static_cast<int>(center_.x);
    int cy = static_cast<int>(center_.y);
    Color c(0.3f, 0.6f, 1.0f, 0.8f);
    
    // 中心十字
    for (int i = -10; i <= 10; ++i) {
        overlay.setPixelSafe(cx + i, cy, c);
        overlay.setPixelSafe(cx, cy + i, c);
    }
}