#pragma once

#include "tool_base.h"

// 自由套索工具
class ToolLasso : public Tool {
public:
    ToolLasso() = default;
    ~ToolLasso() override = default;

    ToolType type() const override { return ToolType::Lasso; }
    const char* name() const override { return "Lasso"; }
    const char* icon() const override { return "L"; }
    const char* cursorHint() const override { return "crosshair"; }

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    bool hasSelection() const override { return hasSelection_; }

private:
    bool drawing_ = false;
    bool hasSelection_ = false;
    std::vector<Vec2> points_;
    Vec2 currentPos_;
};