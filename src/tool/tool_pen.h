#pragma once

#include "tool_base.h"

// 钢笔工具：贝塞尔路径绘制
class ToolPen : public Tool {
public:
    ToolPen() = default;
    ~ToolPen() override = default;

    ToolType type() const override { return ToolType::Pen; }
    const char* name() const override { return "Pen"; }
    const char* icon() const override { return "P"; }
    const char* cursorHint() const override { return "crosshair"; }

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void onKeyDown(int keyCode, bool ctrl, bool shift, bool alt, Canvas& canvas) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

private:
    struct AnchorPoint {
        Vec2 pos;           // 锚点位置
        Vec2 inTangent;     // 入切线控制点
        Vec2 outTangent;    // 出切线控制点
        bool hasInTangent = false;
        bool hasOutTangent = false;
    };
    
    std::vector<AnchorPoint> path_;
    bool drawing_ = false;
    Vec2 currentPos_;
    bool closingPath_ = false; // 点击第一个点闭合
    
    void addAnchor(const Vec2& pos);
    void closePath();
    void drawBezier(CPURenderTarget& overlay, const AnchorPoint& a, const AnchorPoint& b, const Color& c);
};