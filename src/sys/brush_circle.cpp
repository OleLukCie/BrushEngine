#include "brush_circle.h"
#include "../base/base_math.h"

BrushCircle::BrushCircle() {
    settings_.baseSize = 15.0f;
    settings_.minSize = 2.0f;
    settings_.maxSize = 80.0f;
    settings_.pressureSizePower = 0.7f;
    settings_.pressureOpacity = true;
    settings_.baseSpacing = 0.2f;
    settings_.adaptiveSpacing = true;
    settings_.followDirection = false; // 圆笔不跟随方向
    settings_.flattening = 1.0f;       // 正圆
}

void BrushCircle::onStrokeBegin(const InputSample& sample) {
    setState(BrushState::DRAWING);
    currentStroke_.samples.clear();
    currentStroke_.samples.push_back(sample);
    lastSample_ = sample;
    hasLastSample_ = true;
    totalArcLength_ = 0.0f;
}

void BrushCircle::onStrokeMove(const InputSample& sample) {
    if (state_ != BrushState::DRAWING) return;

    currentStroke_.samples.push_back(sample);

    // 计算距离，更新弧长
    float dist = euclideanDistance(lastSample_.x, lastSample_.y, sample.x, sample.y);
    totalArcLength_ += dist;

    lastSample_ = sample;
}

void BrushCircle::onStrokeEnd() {
    if (state_ != BrushState::DRAWING) return;
    // 圆笔不需要 FINALIZING 后处理，直接结束
    setState(BrushState::IDLE);
    hasLastSample_ = false;
}

void BrushCircle::render(Canvas& canvas, const RenderContext& ctx) {
    if (currentStroke_.samples.size() < 2) return;

    initRenderer(canvas);

    // 计算总弧长用于 stroke fade
    float accumulatedLen = 0.0f;
    if (settings_.useStrokeFade) {
        // 重新计算总长度
        totalArcLength_ = 0.0f;
        for (size_t i = 1; i < currentStroke_.samples.size(); ++i) {
            totalArcLength_ += euclideanDistance(
                currentStroke_.samples[i-1].x, currentStroke_.samples[i-1].y,
                currentStroke_.samples[i].x, currentStroke_.samples[i].y);
        }
    }

    // 逐段渲染
    for (size_t i = 1; i < currentStroke_.samples.size(); ++i) {
        const auto& from = currentStroke_.samples[i-1];
        const auto& to = currentStroke_.samples[i];

        if (settings_.useStrokeFade) {
            accumulatedLen += euclideanDistance(from.x, from.y, to.x, to.y);
            settings_.strokeProgress = accumulatedLen / totalArcLength_;
        }

        renderSegment(from, to);
    }
}

void BrushCircle::initRenderer(Canvas& canvas) {
    if (!renderer_) {
        renderer_ = std::make_unique<BrushRenderer>();
    }
    renderer_->setTarget(&canvas.activeLayer().renderTarget());
    renderer_->setSettings(settings_);
}

void BrushCircle::renderSegment(const InputSample& from, const InputSample& to) {
    BrushRenderState fromState, toState;

    fromState.position = Vec2(from.x, from.y);
    fromState.pressure = from.pressure;
    fromState.timestamp = from.timestamp;
    fromState.velocity = 0.0f;
    fromState.direction = 0.0f;

    float dt = to.timestamp - from.timestamp;
    if (dt < 1e-6f) dt = 1.0f;

    toState.position = Vec2(to.x, to.y);
    toState.pressure = to.pressure;
    toState.timestamp = to.timestamp;
    toState.velocity = V(from.x, from.y, to.x, to.y, dt);
    toState.direction = DirectionP(from.x, from.y, to.x, to.y);

    renderer_->renderStroke(fromState.position, toState.position, fromState, toState);
}