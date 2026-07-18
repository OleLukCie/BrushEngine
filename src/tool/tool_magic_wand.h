#pragma once

#include "tool_base.h"

// 魔棒：基于颜色的连续区域选择
class ToolMagicWand : public Tool {
public:
    ToolMagicWand() = default;
    ~ToolMagicWand() override = default;

    ToolType type() const override { return ToolType::MagicWand; }
    const char* name() const override { return "Magic Wand"; }
    const char* icon() const override { return "W"; }
    const char* cursorHint() const override { return "crosshair"; }

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    bool hasSelection() const override { return !selectionMask_.empty(); }

    // 容差设置
    void setTolerance(float t) { tolerance_ = saturate(t); }
    float tolerance() const { return tolerance_; }

    void setContiguous(bool c) { contiguous_ = c; }
    bool isContiguous() const { return contiguous_; }

private:
    float tolerance_ = 0.1f;  // 颜色容差 [0,1]
    bool contiguous_ = true;   // 是否仅选择连续区域
    
    std::vector<bool> selectionMask_; // 与画布等大的选区蒙版
    int maskW_ = 0, maskH_ = 0;
    
    void floodFillSelect(int startX, int startY, Canvas& canvas);
    bool colorMatch(const Color& a, const Color& b) const;
};