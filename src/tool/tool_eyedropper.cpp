#include "tool_eyedropper.h"

void ToolEyedropper::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    sampling_ = true;
    lastColor_ = sampleColor(sample, canvas);
    if (onColorPicked_) onColorPicked_(lastColor_);
}

void ToolEyedropper::onMouseMove(const InputSample& sample, Canvas& canvas) {
    if (!sampling_) return;
    lastColor_ = sampleColor(sample, canvas);
    if (onColorPicked_) onColorPicked_(lastColor_);
}

void ToolEyedropper::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    sampling_ = false;
}

Color ToolEyedropper::sampleColor(const InputSample& sample, Canvas& canvas) {
    int x = static_cast<int>(sample.x);
    int y = static_cast<int>(sample.y);
    
    if (sampleMode_ == SampleMode::SinglePixel) {
        return canvas.getCompositedPixel(x, y);
    }
    
    int radius = (sampleMode_ == SampleMode::Average3x3) ? 1 : 2;
    Color accum;
    int count = 0;
    
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            Color c = canvas.getCompositedPixel(x + dx, y + dy);
            accum = accum + c;
            ++count;
        }
    }
    
    return count > 0 ? accum / static_cast<float>(count) : Color(0, 0, 0, 1);
}

void ToolEyedropper::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    // 绘制当前吸取颜色的预览小方块
}