#include "tool_eraser.h"
#include "../base/base_math.h"

ToolEraser::ToolEraser() {
    settings_.color = Color(1, 1, 1, 1);  // 擦除颜色不重要，混合模式处理
    settings_.blendMode = BlendMode::Erase;
    settings_.baseSize = 20.0f;
    settings_.minSize = 1.0f;
    settings_.maxSize = 100.0f;
    settings_.pressureSizePower = 0.5f;
    settings_.pressureOpacity = true;
    settings_.baseSpacing = 0.25f;
    settings_.adaptiveSpacing = true;
    settings_.followDirection = false;
    settings_.flattening = 1.0f;
}

void ToolEraser::onActivate(Canvas& canvas) {
    active_ = true;
}

void ToolEraser::onDeactivate(Canvas& canvas) {
    active_ = false;
    if (isDrawing_) {
        isDrawing_ = false;
        strokeSamples_.clear();
    }
}

void ToolEraser::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    
    isDrawing_ = true;
    strokeSamples_.clear();
    strokeSamples_.push_back(sample);
    
    initRenderer(canvas);
}

void ToolEraser::onMouseMove(const InputSample& sample, Canvas& canvas) {
    if (!isDrawing_) return;
    
    if (!strokeSamples_.empty()) {
        const auto& last = strokeSamples_.back();
        float dist = euclideanDistance(last.x, last.y, sample.x, sample.y);
        float step = settings_.baseSize * settings_.baseSpacing;
        
        if (dist >= step) {
            renderStrokeSegment(last, sample, canvas);
            strokeSamples_.push_back(sample);
        }
    }
}

void ToolEraser::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0 || !isDrawing_) return;
    
    if (!strokeSamples_.empty()) {
        renderStrokeSegment(strokeSamples_.back(), sample, canvas);
    }
    
    isDrawing_ = false;
    strokeSamples_.clear();
}

void ToolEraser::initRenderer(Canvas& canvas) {
    if (!renderer_) {
        renderer_ = std::make_unique<BrushRenderer>();
    }
    renderer_->setTarget(&canvas.activeLayer().renderTarget());
    renderer_->setSettings(settings_);
}

void ToolEraser::renderStrokeSegment(const InputSample& from, const InputSample& to, Canvas& canvas) {
    BrushRenderState fromState, toState;
    
    fromState.position = Vec2(from.x, from.y);
    fromState.pressure = from.pressure;
    fromState.timestamp = from.timestamp;
    
    float dt = to.timestamp - from.timestamp;
    if (dt < 1e-6f) dt = 1.0f;
    
    toState.position = Vec2(to.x, to.y);
    toState.pressure = to.pressure;
    toState.timestamp = to.timestamp;
    toState.velocity = V(from.x, from.y, to.x, to.y, dt);
    toState.direction = DirectionP(from.x, from.y, to.x, to.y);
    
    renderer_->renderStroke(fromState.position, toState.position, fromState, toState);
}

void ToolEraser::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    // 绘制橡皮擦大小预览圈
    if (!active_) return;
    
    // 这里简化：实际应该在鼠标位置画一个圆
}