#pragma once

#include "tool_base.h"

// 多边形套索：点击创建顶点，双击/回车闭合
class ToolPolyLasso : public Tool {
public:
    ToolPolyLasso() = default;
    ~ToolPolyLasso() override = default;

    ToolType type() const override { return ToolType::PolyLasso; }
    const char* name() const override { return "Polygonal Lasso"; }
    const char* icon() const override { return "L"; }
    const char* cursorHint() const override { return "crosshair"; }

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void onKeyDown(int keyCode, bool ctrl, bool shift, bool alt, Canvas& canvas) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    bool hasSelection() const override { return hasSelection_; }

private:
    std::vector<Vec2> vertices_;
    Vec2 currentPos_;
    bool hasSelection_ = false;
    bool closed_ = false;
};