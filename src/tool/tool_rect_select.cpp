#include "tool_rect_select.h"

void ToolRectSelect::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    
    selecting_ = true;
    dragStart_ = Vec2(sample.x, sample.y);
    dragCurrent_ = dragStart_;
    
    if (!addMode_ && !subtractMode_) {
        hasSelection_ = false; // 新建选区
    }
}

void ToolRectSelect::onMouseMove(const InputSample& sample, Canvas& canvas) {
    if (!selecting_) return;
    dragCurrent_ = Vec2(sample.x, sample.y);
}

void ToolRectSelect::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0 || !selecting_) return;
    
    selecting_ = false;
    
    float x1 = std::min(dragStart_.x, dragCurrent_.x);
    float y1 = std::min(dragStart_.y, dragCurrent_.y);
    float x2 = std::max(dragStart_.x, dragCurrent_.x);
    float y2 = std::max(dragStart_.y, dragCurrent_.y);
    
    if (x2 - x1 > 2.0f && y2 - y1 > 2.0f) {
        selection_ = {x1, y1, x2 - x1, y2 - y1, true};
        hasSelection_ = true;
        if (onSelectionChanged_) onSelectionChanged_(selection_);
    }
}

void ToolRectSelect::onKeyDown(int keyCode, bool ctrl, bool shift, bool alt, Canvas& canvas) {
    addMode_ = shift;
    subtractMode_ = alt;
}

void ToolRectSelect::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    // 绘制选区边框（虚线动画效果）
    if (hasSelection_ && selection_.valid) {
        Color c(1.0f, 1.0f, 1.0f, 0.8f);
        int x = static_cast<int>(selection_.x);
        int y = static_cast<int>(selection_.y);
        int w = static_cast<int>(selection_.w);
        int h = static_cast<int>(selection_.h);
        
        // 简化实线框，实际应用虚线
        for (int i = 0; i < w; ++i) {
            overlay.setPixelSafe(x + i, y, c);
            overlay.setPixelSafe(x + i, y + h, c);
        }
        for (int i = 0; i < h; ++i) {
            overlay.setPixelSafe(x, y + i, c);
            overlay.setPixelSafe(x + w, y + i, c);
        }
    }
    
    // 绘制拖拽中的选区预览
    if (selecting_) {
        float x1 = std::min(dragStart_.x, dragCurrent_.x);
        float y1 = std::min(dragStart_.y, dragCurrent_.y);
        float x2 = std::max(dragStart_.x, dragCurrent_.x);
        float y2 = std::max(dragStart_.y, dragCurrent_.y);
        
        Color c(0.3f, 0.6f, 1.0f, 0.4f);
        // 填充半透明预览
        for (int y = static_cast<int>(y1); y < static_cast<int>(y2); ++y) {
            for (int x = static_cast<int>(x1); x < static_cast<int>(x2); ++x) {
                overlay.setPixelSafe(x, y, c);
            }
        }
    }
}

void ToolRectSelect::deselect() {
    hasSelection_ = false;
    selection_.valid = false;
}

void ToolRectSelect::invertSelection() {
    // 反转选区逻辑（需要画布尺寸）
    // 简化：清除选区
    deselect();
}