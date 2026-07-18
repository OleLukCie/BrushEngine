#pragma once

#include "tool_base.h"

// 移动工具：平移图层内容或选区
class ToolMove : public Tool {
public:
    ToolMove() = default;
    ~ToolMove() override = default;

    ToolType type() const override { return ToolType::Move; }
    const char* name() const override { return "Move"; }
    const char* icon() const override { return "V"; }
    const char* cursorHint() const override { return "move"; }

    void onActivate(Canvas& canvas) override;
    void onDeactivate(Canvas& canvas) override;

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    // 移动模式
    enum class MoveMode {
        Layer,      // 移动整个图层
        Selection,  // 移动选区内容
        Guide       // 移动辅助线
    };
    void setMode(MoveMode mode) { mode_ = mode; }
    MoveMode mode() const { return mode_; }

private:
    MoveMode mode_ = MoveMode::Layer;
    Vec2 dragStartCanvas_;      // 拖拽起始画布坐标
    Vec2 layerStartOffset_;     // 图层原始偏移（预留）
    bool hasSelection_ = false; // 是否有选区可移动
};