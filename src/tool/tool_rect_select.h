#pragma once

#include "tool_base.h"
#include <functional>

// 矩形选区工具
class ToolRectSelect : public Tool {
public:
    ToolRectSelect() = default;
    ~ToolRectSelect() override = default;

    ToolType type() const override { return ToolType::RectSelect; }
    const char* name() const override { return "Rectangular Marquee"; }
    const char* icon() const override { return "M"; }
    const char* cursorHint() const override { return "crosshair"; }

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void onKeyDown(int keyCode, bool ctrl, bool shift, bool alt, Canvas& canvas) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    bool hasSelection() const override { return hasSelection_; }

    // 选区数据
    struct Selection {
        float x = 0.0;
        float y = 0.0;
        float w = 0.0;
        float h = 0.0;
        bool valid = false;
    };
    Selection selection() const { return selection_; }

    // 选区操作
    void deselect();
    void invertSelection();

    // 回调
    using SelectionCallback = std::function<void(const Selection&)>;
    void setOnSelectionChanged(SelectionCallback cb) { onSelectionChanged_ = cb; }

private:
    bool selecting_ = false;
    bool hasSelection_ = false;
    bool addMode_ = false;      // Shift: 添加到选区
    bool subtractMode_ = false; // Alt: 从选区减去
    Vec2 dragStart_;
    Vec2 dragCurrent_;
    Selection selection_;
    SelectionCallback onSelectionChanged_;
};