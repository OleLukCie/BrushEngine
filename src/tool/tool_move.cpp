#include "tool_move.h"
#include "../base/base_math.h"

void ToolMove::onActivate(Canvas& canvas) {
    active_ = true;
}

void ToolMove::onDeactivate(Canvas& canvas) {
    active_ = false;
    dragging_ = false;
}

void ToolMove::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return; // 仅左键
    
    dragging_ = true;
    dragStartCanvas_ = Vec2(sample.x, sample.y);
    
    // 记录图层起始位置（预留扩展：图层变换矩阵）
    // 当前简化：直接操作像素
}

void ToolMove::onMouseMove(const InputSample& sample, Canvas& canvas) {
    if (!dragging_) return;
    
    Vec2 currentCanvas(sample.x, sample.y);
    Vec2 delta = currentCanvas - dragStartCanvas_;
    
    // 这里简化处理：实际应该操作图层的变换矩阵
    // 或创建临时渲染目标进行像素移动
    // 当前版本：标记需要重绘
}

void ToolMove::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    dragging_ = false;
}

void ToolMove::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    if (!dragging_) return;
    
    // 绘制移动预览框
    // 简化：在 overlay 上绘制虚线框
}