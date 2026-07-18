#include "brush_watercolor.h"
#include "../base/base_math.h"

BrushWatercolor::BrushWatercolor() {
    settings_.baseSize = 25.0f;
    settings_.minSize = 5.0f;
    settings_.maxSize = 100.0f;
    settings_.pressureSizePower = 0.6f;
    settings_.pressureOpacity = true;
    settings_.baseSpacing = 0.3f;
    settings_.adaptiveSpacing = true;
    settings_.followDirection = true;
    settings_.flattening = 1.0f;
    settings_.useVelocityOpacity = true;
    settings_.velocityOpacityDecay = 0.6f;
    settings_.blendMode = BlendMode::Normal;
    settings_.opacity = 0.3f; // 水彩默认半透明
}

void BrushWatercolor::onStrokeBegin(const InputSample& sample) {
    setState(BrushState::DRAWING);
    currentStroke_.samples.clear();
    currentStroke_.samples.push_back(sample);
    lastSample_ = sample;
    hasLastSample_ = true;
    totalArcLength_ = 0.0f;
    wetMask_.clear();
}

void BrushWatercolor::onStrokeMove(const InputSample& sample) {
    if (state_ != BrushState::DRAWING) return;
    currentStroke_.samples.push_back(sample);

    float dist = euclideanDistance(lastSample_.x, lastSample_.y, sample.x, sample.y);
    totalArcLength_ += dist;
    lastSample_ = sample;
}

void BrushWatercolor::onStrokeEnd() {
    if (state_ != BrushState::DRAWING) return;
    // 进入 FINALIZING 状态，执行扩散结算
    setState(BrushState::FINALIZING);
}

void BrushWatercolor::render(Canvas& canvas, const RenderContext& ctx) {
    if (currentStroke_.samples.size() < 2) {
        if (state_ == BrushState::FINALIZING) {
            applyDiffusion(canvas);
            setState(BrushState::IDLE);
        }
        return;
    }

    initRenderer(canvas);

    // 如果是 FINALIZING 状态，先执行扩散
    if (state_ == BrushState::FINALIZING) {
        // 先完成剩余绘制
        for (size_t i = 1; i < currentStroke_.samples.size(); ++i) {
            renderSegment(currentStroke_.samples[i-1], currentStroke_.samples[i]);
        }
        applyDiffusion(canvas);
        setState(BrushState::IDLE);
        return;
    }

    // 正常绘制
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

void BrushWatercolor::initRenderer(Canvas& canvas) {
    if (!renderer_) {
        renderer_ = std::make_unique<BrushRenderer>();
    }
    renderer_->setTarget(&canvas.activeLayer().renderTarget());
    renderer_->setSettings(settings_);
}

void BrushWatercolor::renderSegment(const InputSample& from, const InputSample& to) {
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

    // 水彩：速度越快越透明（飞白）
    float savedOpacity = settings_.opacity;
    if (settings_.useVelocityOpacity) {
        float vel = toState.velocity;
        float opacityMod = velocityOpacity(settings_.opacity, vel, settings_.velocityOpacityDecay);
        settings_.opacity = opacityMod * (1.0f - waterAmount_ * 0.3f); // 水量多稍微更不透明
        renderer_->setSettings(settings_);
    }

    renderer_->renderStroke(fromState.position, toState.position, fromState, toState);

    settings_.opacity = savedOpacity;
    renderer_->setSettings(settings_);
}

void BrushWatercolor::applyDiffusion(Canvas& canvas) {
    // 获取当前图层
    auto& layer = canvas.activeLayer();
    int w = layer.width();
    int h = layer.height();

    if (w <= 0 || h <= 0) return;

    // 将图层转换为灰度浮点数据作为高度图/湿润蒙版
    std::vector<float> grayMap(w * h, 0.0f);
    const auto& pixels = layer.renderTarget().pixels();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            const Color& c = pixels[idx];
            // 使用亮度作为湿润度指标
            grayMap[idx] = (c.r * 0.299f + c.g * 0.587f + c.b * 0.114f) * c.a;
        }
    }

    // 应用 Perona-Malik 扩散
    // 根据水量调整 K 和迭代次数
    float k = diffusionK_ * (0.5f + waterAmount_);
    int iter = static_cast<int>(diffusionIter_ * (0.3f + waterAmount_ * 0.7f));
    float lambda = diffusionLambda_ * waterAmount_;

    PMDiffusion(grayMap, w, h, k, lambda, iter);

    // 将扩散后的结果混合回图层
    // 这里简化为：根据扩散后的灰度值调整原图层的 alpha
    auto& mutablePixels = layer.renderTarget().pixels();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            float diffused = saturate(grayMap[idx]);
            if (diffused > 0.01f) {
                Color& c = mutablePixels[idx];
                // 晕染效果：稍微模糊边缘，降低对比度
                float blendFactor = diffused * waterAmount_ * 0.3f;
                c.a = saturate(c.a + blendFactor * 0.1f);
            }
        }
    }
}