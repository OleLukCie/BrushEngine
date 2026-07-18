#pragma once

#include "brush.h"
#include <vector>

// 水彩笔刷：使用 PMDiffusion + velocityOpacity 做晕染和飞白
// FINALIZING 阶段执行扩散结算
class BrushWatercolor : public Brush {
public:
    BrushWatercolor();
    ~BrushWatercolor() override = default;

    const char* brushName() const override { return "Watercolor"; }

    void onStrokeBegin(const InputSample& sample) override;
    void onStrokeMove(const InputSample& sample) override;
    void onStrokeEnd() override;
    void render(Canvas& canvas, const RenderContext& ctx) override;

    // 水彩参数
    void setDiffusionStrength(float k) { diffusionK_ = k; }
    float diffusionStrength() const { return diffusionK_; }

    void setDiffusionIterations(int iter) { diffusionIter_ = iter; }
    int diffusionIterations() const { return diffusionIter_; }

    void setWaterAmount(float amount) { waterAmount_ = saturate(amount); }
    float waterAmount() const { return waterAmount_; }

private:
    // 水彩参数
    float diffusionK_ = 15.0f;      // Perona-Malik 扩散系数 K
    float diffusionLambda_ = 0.15f; // 扩散步长
    int diffusionIter_ = 10;        // 扩散迭代次数
    float waterAmount_ = 0.5f;      // 水量 [0,1]，影响扩散范围

    // 渲染数据
    std::unique_ptr<BrushRenderer> renderer_;
    InputSample lastSample_;
    bool hasLastSample_ = false;
    float totalArcLength_ = 0.0f;

    // 用于 FINALIZING 的临时数据
    std::vector<float> wetMask_;    // 湿润区域蒙版
    int wetMaskW_ = 0, wetMaskH_ = 0;

    void initRenderer(Canvas& canvas);
    void renderSegment(const InputSample& from, const InputSample& to);
    void applyDiffusion(Canvas& canvas);
};