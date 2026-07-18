#pragma once

#include "tool_base.h"
#include <functional>

// 旋转画布：拖拽旋转视图，双击复位
class ToolRotateCanvas : public Tool {
public:
    ToolRotateCanvas() = default;
    ~ToolRotateCanvas() override = default;

    ToolType type() const override { return ToolType::RotateCanvas; }
    const char* name() const override { return "Rotate Canvas"; }
    const char* icon() const override { return "R"; }
    const char* cursorHint() const override { return "rotate"; }

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    // 旋转回调
    using RotateCallback = std::function<void(float angleRad)>;
    void setOnRotate(RotateCallback cb) { onRotate_ = cb; }

private:
    bool rotating_ = false;
    Vec2 center_;           // 画布中心
    Vec2 dragStart_;
    float startAngle_ = 0.0f;
    float currentAngle_ = 0.0f;
    RotateCallback onRotate_;
};