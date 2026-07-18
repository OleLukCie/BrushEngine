#pragma once

#include "brush.h"

// 马克笔：多层半透明叠加，用 lambertDiffuseColor 做简单光照体积感
class BrushMarker : public Brush {
public:
    BrushMarker();
    ~BrushMarker() override = default;

    const char* brushName() const override { return "Marker"; }

    void onStrokeBegin(const InputSample& sample) override;
    void onStrokeMove(const InputSample& sample) override;
    void onStrokeEnd() override;
    void render(Canvas& canvas, const RenderContext& ctx) override;

    // 马克笔参数
    void setLayerCount(int count) { layerCount_ = std::max(1, count); }
    int layerCount() const { return layerCount_; }

    void setLightDirection(const Vec3& dir) { lightDir_ = dir.normalized(); }
    Vec3 lightDirection() const { return lightDir_; }

private:
    int layerCount_ = 3;            // 叠加层数
    Vec3 lightDir_ = Vec3(0, 0, 1); // 默认正面光

    std::unique_ptr<BrushRenderer> renderer_;
    InputSample lastSample_;
    bool hasLastSample_ = false;
    float totalArcLength_ = 0.0f;

    void initRenderer(Canvas& canvas);
    void renderSegment(const InputSample& from, const InputSample& to);
};