#pragma once

#include "tool_base.h"

// 裁剪工具
class ToolCrop : public Tool {
public:
    ToolCrop() = default;
    ~ToolCrop() override = default;

    ToolType type() const override { return ToolType::Crop; }
    const char* name() const override { return "Crop"; }
    const char* icon() const override { return "C"; }
    const char* cursorHint() const override { return "crosshair"; }

    void onActivate(Canvas& canvas) override;
    void onDeactivate(Canvas& canvas) override;

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void onKeyDown(int keyCode, bool ctrl, bool shift, bool alt, Canvas& canvas) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    // 应用裁剪
    void applyCrop(Canvas& canvas);
    void cancelCrop();

    bool isCropping() const { return cropping_; }

private:
    bool cropping_ = false;
    Vec2 dragStart_;
    Vec2 dragCurrent_;
    
    struct CropRect {
        float x = 0.0;
        float y = 0.0;
        float w = 0.0;
        float h = 0.0;
        bool valid = false;
    } cropRect_;
    
    // 拖拽控制点
    enum class Handle {
        None, TopLeft, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left, Center
    };
    Handle activeHandle_ = Handle::None;
    
    Handle hitTest(float x, float y) const;
};