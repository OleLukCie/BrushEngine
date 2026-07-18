#pragma once

#include "brush.h"

// 基础圆笔：最简单的硬边/软边圆笔
// 用于验证渲染管线，也是其他笔刷的基础参考实现
class BrushCircle : public Brush {
public:
    BrushCircle();
    ~BrushCircle() override = default;

    const char* brushName() const override { return "Circle"; }

    void onStrokeBegin(const InputSample& sample) override;
    void onStrokeMove(const InputSample& sample) override;
    void onStrokeEnd() override;
    void render(Canvas& canvas, const RenderContext& ctx) override;

    // 设置软边/硬边
    void setSoftEdge(bool soft) { softEdge_ = soft; }
    bool isSoftEdge() const { return softEdge_; }

private:
    bool softEdge_ = true;  // true=软边（高斯衰减），false=硬边

    // 内部渲染器
    std::unique_ptr<BrushRenderer> renderer_;

    // 上一个采样点
    InputSample lastSample_;
    bool hasLastSample_ = false;

    // 笔触总长度（用于 stroke fade）
    float totalArcLength_ = 0.0f;

    void initRenderer(Canvas& canvas);
    void renderSegment(const InputSample& from, const InputSample& to);
};