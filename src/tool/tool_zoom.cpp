#include "tool_zoom.h"

void ToolZoom::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    
    zoomOut_ = false; // 检查 Alt 状态由外部输入层处理，这里简化
    dragging_ = true;
    dragStart_ = Vec2(sample.x, sample.y);
    dragCurrent_ = dragStart_;
}

void ToolZoom::onMouseMove(const InputSample& sample, Canvas& canvas) {
    if (!dragging_) return;
    dragCurrent_ = Vec2(sample.x, sample.y);
}

void ToolZoom::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0 || !dragging_) return;
    
    Vec2 end(sample.x, sample.y);
    float dx = std::abs(end.x - dragStart_.x);
    float dy = std::abs(end.y - dragStart_.y);
    
    if (dx < 5.0f && dy < 5.0f) {
        // 点击模式：以鼠标为中心缩放
        float factor = zoomOut_ ? (1.0f / ZOOM_STEP) : ZOOM_STEP;
        if (onZoom_) onZoom_(factor, end);
    } else {
        // 框选模式：缩放到选中区域
        Vec2 center((dragStart_.x + end.x) * 0.5f, (dragStart_.y + end.y) * 0.5f);
        float canvasW = static_cast<float>(canvas.width());
        float canvasH = static_cast<float>(canvas.height());
        float scaleX = canvasW / dx;
        float scaleY = canvasH / dy;
        float newScale = std::min(scaleX, scaleY);
        if (onZoom_) onZoom_(newScale, center);
    }
    
    dragging_ = false;
}

void ToolZoom::onMouseWheel(float delta, Canvas& canvas) {
    float factor = delta > 0 ? ZOOM_STEP : (1.0f / ZOOM_STEP);
    Vec2 center(canvas.width() * 0.5f, canvas.height() * 0.5f);
    if (onZoom_) onZoom_(factor, center);
}

void ToolZoom::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    if (!dragging_) return;
    
    // 绘制框选矩形
    float x = std::min(dragStart_.x, dragCurrent_.x);
    float y = std::min(dragStart_.y, dragCurrent_.y);
    float w = std::abs(dragCurrent_.x - dragStart_.x);
    float h = std::abs(dragCurrent_.y - dragStart_.y);
    
    // 使用 overlay 绘制虚线框
    Color c(0.3f, 0.6f, 1.0f, 0.8f);
    // 简化：实线框
    for (int i = 0; i < static_cast<int>(w); ++i) {
        overlay.setPixelSafe(static_cast<int>(x + i), static_cast<int>(y), c);
        overlay.setPixelSafe(static_cast<int>(x + i), static_cast<int>(y + h), c);
    }
    for (int i = 0; i < static_cast<int>(h); ++i) {
        overlay.setPixelSafe(static_cast<int>(x), static_cast<int>(y + i), c);
        overlay.setPixelSafe(static_cast<int>(x + w), static_cast<int>(y + i), c);
    }
}