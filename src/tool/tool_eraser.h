#pragma once

#include "tool_base.h"
#include "../sys/brush.h"

// 橡皮擦本质上是使用 Erase 混合模式的笔刷
class ToolEraser : public Tool {
public:
    ToolEraser();
    ~ToolEraser() override = default;

    ToolType type() const override { return ToolType::Eraser; }
    const char* name() const override { return "Eraser"; }
    const char* icon() const override { return "E"; }
    const char* cursorHint() const override { return "crosshair"; }

    void onActivate(Canvas& canvas) override;
    void onDeactivate(Canvas& canvas) override;

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    // 橡皮擦设置
    void setSize(float size) { settings_.baseSize = size; }
    float size() const { return settings_.baseSize; }
    
    void setHardness(float h) { hardness_ = saturate(h); }
    float hardness() const { return hardness_; }

    BrushSettings& brushSettings() { return settings_; }

private:
    BrushSettings settings_;
    float hardness_ = 1.0f;  // 硬度：1=硬边，0=软边
    
    std::unique_ptr<BrushRenderer> renderer_;
    std::vector<InputSample> strokeSamples_;
    bool isDrawing_ = false;

    void initRenderer(Canvas& canvas);
    void renderStrokeSegment(const InputSample& from, const InputSample& to, Canvas& canvas);
};