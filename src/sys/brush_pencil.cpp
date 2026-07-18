#include "brush_pencil.h"
#include "../base/base_math.h"

BrushPencil::BrushPencil() {
    settings_.baseSize = 3.0f;
    settings_.minSize = 0.5f;
    settings_.maxSize = 20.0f;
    settings_.pressureSizePower = 0.8f;
    settings_.pressureOpacity = true;
    settings_.baseSpacing = 0.15f;
    settings_.adaptiveSpacing = true;
    settings_.followDirection = true;
    settings_.flattening = 0.85f;  // 略微压扁，更像铅笔
    settings_.useVelocityOpacity = true;
    settings_.velocityOpacityDecay = 0.3f;
    settings_.blendMode = BlendMode::Multiply; // 铅笔用正片叠底更有质感
}

void BrushPencil::onStrokeBegin(const InputSample& sample) {
    setState(BrushState::DRAWING);
    currentStroke_.samples.clear();
    currentStroke_.samples.push_back(sample);
    lastSample_ = sample;
    hasLastSample_ = true;
    totalArcLength_ = 0.0f;
}

void BrushPencil::onStrokeMove(const InputSample& sample) {
    if (state_ != BrushState::DRAWING) return;
    currentStroke_.samples.push_back(sample);

    float dist = euclideanDistance(lastSample_.x, lastSample_.y, sample.x, sample.y);
    totalArcLength_ += dist;
    lastSample_ = sample;
}

void BrushPencil::onStrokeEnd() {
    if (state_ != BrushState::DRAWING) return;
    setState(BrushState::IDLE);
    hasLastSample_ = false;
}

void BrushPencil::render(Canvas& canvas, const RenderContext& ctx) {
    if (currentStroke_.samples.size() < 2) return;

    initRenderer(canvas);
    ensureNoiseTexture();

    float accumulatedLen = 0.0f;
    if (settings_.useStrokeFade) {
        totalArcLength_ = 0.0f;
        for (size_t i = 1; i < currentStroke_.samples.size(); ++i) {
            totalArcLength_ += euclideanDistance(
                currentStroke_.samples[i-1].x, currentStroke_.samples[i-1].y,
                currentStroke_.samples[i].x, currentStroke_.samples[i].y);
        }
    }

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

void BrushPencil::initRenderer(Canvas& canvas) {
    if (!renderer_) {
        renderer_ = std::make_unique<BrushRenderer>();
    }
    renderer_->setTarget(&canvas.activeLayer().renderTarget());
    renderer_->setSettings(settings_);
}

void BrushPencil::ensureNoiseTexture() {
    if (!noiseTexture_ || noiseTexture_->empty()) {
        // 生成 256x256 的蓝噪声纹理
        noiseTexture_ = std::make_unique<Texture>(
            Texture::generateBlueNoise(256, 256, grainIntensity_));
        settings_.texture = noiseTexture_.get();
        settings_.textureScale = 2.0f;
    }
}

void BrushPencil::renderSegment(const InputSample& from, const InputSample& to) {
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

    // 铅笔特有的：根据速度调整不透明度（飞白效果）
    if (settings_.useVelocityOpacity) {
        float vel = toState.velocity;
        float opacityMod = velocityOpacity(settings_.opacity, vel, settings_.velocityOpacityDecay);
        // 临时修改 opacity 渲染这一段
        float savedOpacity = settings_.opacity;
        settings_.opacity = opacityMod;
        renderer_->setSettings(settings_);
        renderer_->renderStroke(fromState.position, toState.position, fromState, toState);
        settings_.opacity = savedOpacity;
        renderer_->setSettings(settings_);
    } else {
        renderer_->renderStroke(fromState.position, toState.position, fromState, toState);
    }
}