#pragma once

#include "tool_base.h"
#include <functional>

// 吸管工具：吸取画布颜色
class ToolEyedropper : public Tool {
public:
    ToolEyedropper() = default;
    ~ToolEyedropper() override = default;

    ToolType type() const override { return ToolType::Eyedropper; }
    const char* name() const override { return "Eyedropper"; }
    const char* icon() const override { return "I"; }
    const char* cursorHint() const override { return "crosshair"; }

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    // 吸取模式
    enum class SampleMode {
        SinglePixel,    // 单像素
        Average3x3,     // 3x3 平均
        Average5x5      // 5x5 平均
    };
    void setSampleMode(SampleMode mode) { sampleMode_ = mode; }
    SampleMode sampleMode() const { return sampleMode_; }

    // 获取上次吸取的颜色
    Color lastPickedColor() const { return lastColor_; }

    // 回调：颜色变化时通知
    using ColorCallback = std::function<void(const Color&)>;
    void setOnColorPicked(ColorCallback cb) { onColorPicked_ = cb; }

private:
    SampleMode sampleMode_ = SampleMode::SinglePixel;
    Color lastColor_ = Color(0, 0, 0, 1);
    ColorCallback onColorPicked_;
    bool sampling_ = false;

    Color sampleColor(const InputSample& sample, Canvas& canvas);
};