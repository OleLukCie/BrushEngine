#pragma once

#include "brush.h"
#include "../render/texture.h"

// 铅笔笔刷：使用 blueNoise + ellipseCoverage 做颗粒感
class BrushPencil : public Brush {
public:
    BrushPencil();
    ~BrushPencil() override = default;

    const char* brushName() const override { return "Pencil"; }

    void onStrokeBegin(const InputSample& sample) override;
    void onStrokeMove(const InputSample& sample) override;
    void onStrokeEnd() override;
    void render(Canvas& canvas, const RenderContext& ctx) override;

    // 设置颗粒强度 [0,1]
    void setGrainIntensity(float intensity) { grainIntensity_ = saturate(intensity); }
    float grainIntensity() const { return grainIntensity_; }

private:
    float grainIntensity_ = 0.6f;   // 颗粒强度

    std::unique_ptr<BrushRenderer> renderer_;
    std::unique_ptr<Texture> noiseTexture_;  // 蓝噪声纹理

    InputSample lastSample_;
    bool hasLastSample_ = false;
    float totalArcLength_ = 0.0f;

    void initRenderer(Canvas& canvas);
    void ensureNoiseTexture();
    void renderSegment(const InputSample& from, const InputSample& to);
};