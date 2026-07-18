#pragma once

#include "tool_base.h"

// 椭圆选区工具（Shift+M）
class ToolEllipseSelect : public Tool {
public:
    ToolEllipseSelect() = default;
    ~ToolEllipseSelect() override = default;

    ToolType type() const override { return ToolType::EllipseSelect; }
    const char* name() const override { return "Elliptical Marquee"; }
    const char* icon() const override { return "M"; }
    const char* cursorHint() const override { return "crosshair"; }

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    bool hasSelection() const override { return hasSelection_; }

private:
    bool selecting_ = false;
    bool hasSelection_ = false;
    Vec2 center_;
    float radiusX_ = 0.0f;
    float radiusY_ = 0.0f;
    Vec2 dragStart_;
    Vec2 dragCurrent_;
};