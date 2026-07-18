#include "brush_marker.h"
#include "../base/base_math.h"

BrushMarker::BrushMarker() {
    settings_.baseSize = 20.0f;
    settings_.minSize = 3.0f;
    settings_.maxSize = 60.0f;
    settings_.pressureSizePower = 0.5f;
    settings_.pressureOpacity = true;
    settings_.baseSpacing = 0.25f;
    settings_.adaptiveSpacing = true;
    settings_.followDirection = true;
    settings_.flattening = 0.7f;  // 马克笔是扁的
    settings_.rotation = 0.0f;
    settings_.blendMode = BlendMode::Normal;
    settings_.opacity = 0.25f; // 单层很透明，靠多层叠加
}

void BrushMarker::onStrokeBegin(const InputSample& sample) {
    setState(BrushState::DRAWING);
    currentStroke_.samples.clear();
    currentStroke_.samples.push_back(sample);
    lastSample_ = sample;
    hasLastSample_ = true;
    totalArcLength_ = 0.0f;
}

void BrushMarker::onStrokeMove(const InputSample& sample) {
    if (state_ != BrushState::DRAWING) return;
    currentStroke_.samples.push_back(sample);

    float dist = euclideanDistance(lastSample_.x, lastSample_.y, sample.x, sample.y);
    totalArcLength_ += dist;
    lastSample_ = sample;
}

void BrushMarker::onStrokeEnd() {
    if (state_ != BrushState::DRAWING) return;
    setState(BrushState::IDLE);
    hasLastSample_ = false;
}

void BrushMarker::render(Canvas& canvas, const RenderContext& ctx) {
    if (currentStroke_.samples.size() < 2) return;

    initRenderer(canvas);

    float accumulatedLen = 0.0f;
    if (settings_.useStrokeFade) {
        totalArcLength_ = 0.0f;
        for (size_t i = 1; i < currentStroke_.samples.size(); ++i) {
            totalArcLength_ += euclideanDistance(
                currentStroke_.samples[i-1].x, currentStroke_.samples[i-1].y,
                currentStroke_.samples[i].x, currentStroke_.samples[i].y);
        }
    }

    // 马克笔：多层叠加渲染
    for (int layer = 0; layer < layerCount_; ++layer) {
        // 每层略微偏移，模拟墨水渗透不均匀
        float layerOffset = (layer - layerCount_ / 2.0f) * 0.5f;

        for (size_t i = 1; i < currentStroke_.samples.size(); ++i) {
            const auto& from = currentStroke_.samples[i-1];
            const auto& to = currentStroke_.samples[i];

            if (settings_.useStrokeFade) {
                accumulatedLen += euclideanDistance(from.x, from.y, to.x, to.y);
                settings_.strokeProgress = accumulatedLen / totalArcLength_;
            }

            // 每层使用略微不同的参数
            float savedOpacity = settings_.opacity;
            float savedSize = settings_.baseSize;

            // 层数越多，每层越淡，整体越大（墨水扩散）
            settings_.opacity = savedOpacity * (1.0f - layer * 0.15f);
            settings_.baseSize = savedSize * (1.0f + layer * 0.1f);

            renderer_->setSettings(settings_);

            InputSample fromOffset = from;
            InputSample toOffset = to;
            fromOffset.x += layerOffset;
            fromOffset.y += layerOffset;
            toOffset.x += layerOffset;
            toOffset.y += layerOffset;

            renderSegment(fromOffset, toOffset);

            settings_.opacity = savedOpacity;
            settings_.baseSize = savedSize;
        }
    }

    renderer_->setSettings(settings_);
}

void BrushMarker::initRenderer(Canvas& canvas) {
    if (!renderer_) {
        renderer_ = std::make_unique<BrushRenderer>();
    }
    renderer_->setTarget(&canvas.activeLayer().renderTarget());
    renderer_->setSettings(settings_);
}

void BrushMarker::renderSegment(const InputSample& from, const InputSample& to) {
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

    // 马克笔体积感：根据笔尖方向计算法线，应用 Lambert 漫反射
    // 简化处理：根据压感模拟"墨水堆积"的高度，产生明暗变化
    float pressureHeight = from.pressure * 0.3f;
    Vec3 normal(0, 0, 1);
    // 根据运笔方向倾斜法线，模拟侧光效果
    float dirRad = toState.direction;
    normal.x = std::cos(dirRad) * pressureHeight;
    normal.y = std::sin(dirRad) * pressureHeight;
    normal = normal.normalized();

    // 使用 lambertDiffuseColor 计算光照后的颜色
    Color litColor = lambertDiffuseColor(normal, lightDir_, settings_.color);

    // 临时替换颜色
    Color savedColor = settings_.color;
    settings_.color = litColor;
    renderer_->setSettings(settings_);

    renderer_->renderStroke(fromState.position, toState.position, fromState, toState);

    settings_.color = savedColor;
    renderer_->setSettings(settings_);
}