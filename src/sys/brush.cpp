#include "brush.h"
#include "../base/base_math.h"

// ========== Stroke ==========

void Stroke::getBounds(float& minX, float& minY, float& maxX, float& maxY) const {
    if (samples.empty()) {
        minX = minY = maxX = maxY = 0.0f;
        return;
    }
    minX = maxX = samples[0].x;
    minY = maxY = samples[0].y;
    for (const auto& s : samples) {
        minX = std::min(minX, s.x);
        maxX = std::max(maxX, s.x);
        minY = std::min(minY, s.y);
        maxY = std::max(maxY, s.y);
    }
}

// ========== Brush ==========

Brush::Brush() {
    settings_.color = Color(0, 0, 0, 1);
    settings_.baseSize = 10.0f;
    settings_.minSize = 1.0f;
    settings_.maxSize = 50.0f;
    settings_.opacity = 1.0f;
    settings_.pressureSizePower = 0.5f;
    settings_.pressureOpacity = true;
    settings_.velocityOpacityDecay = 0.5f;
    settings_.useVelocityOpacity = false;
    settings_.flattening = 1.0f;
    settings_.rotation = 0.0f;
    settings_.followDirection = true;
    settings_.texture = nullptr;
    settings_.textureScale = 1.0f;
    settings_.textureRotation = 0.0f;
    settings_.blendMode = BlendMode::Normal;
    settings_.baseSpacing = 0.25f;
    settings_.adaptiveSpacing = true;
    settings_.useStrokeFade = false;
    settings_.fadeStart = 0.0f;
    settings_.fadeEnd = 1.0f;
    settings_.strokeProgress = 0.5f;
}

void Brush::setState(BrushState newState) {
    state_ = newState;
}

void Brush::transitionTo(BrushState newState) {
    // 简单的状态机校验
    // IDLE -> DRAWING -> FINALIZING -> IDLE
    // 或者 IDLE -> DRAWING -> IDLE（直接结束，跳过 FINALIZING）
    state_ = newState;
}

void Brush::appendSample(const InputSample& sample) {
    currentStroke_.samples.push_back(sample);
}

void Brush::forceEnd() {
    currentStroke_.samples.clear();
    state_ = BrushState::IDLE;
}

BrushRenderState Brush::sampleToBrushState(const InputSample& sample, const InputSample& prev) const {
    BrushRenderState state;
    state.position = Vec2(sample.x, sample.y);
    state.pressure = sample.pressure;
    state.timestamp = sample.timestamp;

    float dx = sample.x - prev.x;
    float dy = sample.y - prev.y;
    float dt = sample.timestamp - prev.timestamp;
    if (dt < 1e-6f) dt = 1.0f; // 防止除零

    state.velocity = V(prev.x, prev.y, sample.x, sample.y, dt);
    state.direction = DirectionP(prev.x, prev.y, sample.x, sample.y);

    return state;
}