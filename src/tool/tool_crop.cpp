#include "tool_crop.h"
#define NOMINMAX
#include <windows.h>

void ToolCrop::onActivate(Canvas& canvas) {
    active_ = true;
    // 默认选中整个画布
    cropRect_ = {0, 0, static_cast<float>(canvas.width()), static_cast<float>(canvas.height()), true};
}

void ToolCrop::onDeactivate(Canvas& canvas) {
    active_ = false;
    cropping_ = false;
}

void ToolCrop::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    
    if (!cropRect_.valid) {
        // 新建裁剪区
        cropping_ = true;
        dragStart_ = Vec2(sample.x, sample.y);
        dragCurrent_ = dragStart_;
    } else {
        // 调整现有裁剪区
        activeHandle_ = hitTest(sample.x, sample.y);
        if (activeHandle_ != Handle::None) {
            cropping_ = true;
            dragStart_ = Vec2(sample.x, sample.y);
        }
    }
}

void ToolCrop::onMouseMove(const InputSample& sample, Canvas& canvas) {
    if (!cropping_) return;
    dragCurrent_ = Vec2(sample.x, sample.y);
    
    // 根据控制点调整裁剪矩形
    // 简化：仅支持新建模式
    if (!cropRect_.valid || activeHandle_ == Handle::Center) {
        float x1 = std::min(dragStart_.x, dragCurrent_.x);
        float y1 = std::min(dragStart_.y, dragCurrent_.y);
        float x2 = std::max(dragStart_.x, dragCurrent_.x);
        float y2 = std::max(dragStart_.y, dragCurrent_.y);
        cropRect_ = {x1, y1, x2 - x1, y2 - y1, true};
    }
}

void ToolCrop::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    cropping_ = false;
    activeHandle_ = Handle::None;
}

void ToolCrop::onKeyDown(int keyCode, bool ctrl, bool shift, bool alt, Canvas& canvas) {
    if (keyCode == VK_RETURN) {
        applyCrop(canvas);
    } else if (keyCode == VK_ESCAPE) {
        cancelCrop();
    }
}

void ToolCrop::applyCrop(Canvas& canvas) {
    if (!cropRect_.valid) return;
    
    int newW = static_cast<int>(cropRect_.w);
    int newH = static_cast<int>(cropRect_.h);
    if (newW <= 0 || newH <= 0) return;
    
    // 裁剪所有图层
    // 简化：实际应遍历图层，截取对应区域
    cropRect_.valid = false;
    active_ = false;
}

void ToolCrop::cancelCrop() {
    cropRect_.valid = false;
    cropping_ = false;
}

ToolCrop::Handle ToolCrop::hitTest(float x, float y) const {
    if (!cropRect_.valid) return Handle::None;
    
    float hx = cropRect_.x;
    float hy = cropRect_.y;
    float hw = cropRect_.w;
    float hh = cropRect_.h;
    float handleSize = 6.0f;
    
    // 简化：仅检测中心和整体
    if (x >= hx && x <= hx + hw && y >= hy && y <= hy + hh) {
        return Handle::Center;
    }
    return Handle::None;
}

void ToolCrop::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    if (!cropRect_.valid && !cropping_) return;
    
    float x, y, w, h;
    if (cropping_ && !cropRect_.valid) {
        x = std::min(dragStart_.x, dragCurrent_.x);
        y = std::min(dragStart_.y, dragCurrent_.y);
        w = std::abs(dragCurrent_.x - dragStart_.x);
        h = std::abs(dragCurrent_.y - dragStart_.y);
    } else {
        x = cropRect_.x; y = cropRect_.y;
        w = cropRect_.w; h = cropRect_.h;
    }
    
    // 绘制暗角（裁剪区外）
    Color dim(0, 0, 0, 0.5f);
    // 简化：仅绘制边框
    
    // 绘制裁剪框
    Color border(0.3f, 0.6f, 1.0f, 1.0f);
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int iw = static_cast<int>(w);
    int ih = static_cast<int>(h);
    
    for (int i = 0; i < iw; ++i) {
        overlay.setPixelSafe(ix + i, iy, border);
        overlay.setPixelSafe(ix + i, iy + ih, border);
    }
    for (int i = 0; i < ih; ++i) {
        overlay.setPixelSafe(ix, iy + i, border);
        overlay.setPixelSafe(ix + iw, iy + i, border);
    }
    
    // 三分线
    Color guide(1, 1, 1, 0.5f);
    int x3 = ix + iw / 3;
    int x6 = ix + iw * 2 / 3;
    int y3 = iy + ih / 3;
    int y6 = iy + ih * 2 / 3;
    
    for (int i = 0; i < ih; ++i) {
        overlay.setPixelSafe(x3, iy + i, guide);
        overlay.setPixelSafe(x6, iy + i, guide);
    }
    for (int i = 0; i < iw; ++i) {
        overlay.setPixelSafe(ix + i, y3, guide);
        overlay.setPixelSafe(ix + i, y6, guide);
    }
}