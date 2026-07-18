#pragma once

#include "input_sample.h"
#include "../render/canvas.h"
#include "../render/brush_renderer.h"
#include "../base/base_types.h"
#include <vector>
#include <memory>

// 笔刷状态机
enum class BrushState {
    IDLE,       // 空闲
    DRAWING,    // 绘制中
    FINALIZING  // 收尾处理（如水彩扩散结算）
};

// 渲染上下文
struct RenderContext {
    float deltaTime = 0.0f;     // 帧间隔时间
    float canvasScale = 1.0f;   // 画布缩放
    Vec2 canvasOffset;          // 画布偏移
};

// 笔触数据：存储原始输入序列，支持撤销重放和矢量导出
struct Stroke {
    std::vector<InputSample> samples;   // 原始输入点序列
    std::vector<BrushState> states;      // 每段的状态（用于分段渲染）

    bool empty() const { return samples.empty(); }
    size_t pointCount() const { return samples.size(); }

    // 获取笔触包围盒
    void getBounds(float& minX, float& minY, float& maxX, float& maxY) const;
};

// 笔刷基类：纯虚接口
class Brush {
public:
    Brush();
    virtual ~Brush() = default;

    // 禁用拷贝，允许移动
    Brush(const Brush&) = delete;
    Brush& operator=(const Brush&) = delete;
    Brush(Brush&&) = default;
    Brush& operator=(Brush&&) = default;

    // ===== 核心生命周期接口 =====

    // 笔触开始
    virtual void onStrokeBegin(const InputSample& sample) = 0;

    // 笔触移动
    virtual void onStrokeMove(const InputSample& sample) = 0;

    // 笔触结束
    virtual void onStrokeEnd() = 0;

    // 渲染到画布
    virtual void render(Canvas& canvas, const RenderContext& ctx) = 0;

    // ===== 状态查询 =====

    BrushState state() const { return state_; }
    bool isDrawing() const { return state_ == BrushState::DRAWING; }
    bool isFinalizing() const { return state_ == BrushState::FINALIZING; }

    // 获取当前笔触（用于撤销/重放/导出）
    const Stroke& currentStroke() const { return currentStroke_; }

    // 获取画笔渲染器配置（子类可覆盖）
    virtual BrushSettings& brushSettings() { return settings_; }
    virtual const BrushSettings& brushSettings() const { return settings_; }

    // 设置画笔配置
    virtual void setBrushSettings(const BrushSettings& settings) { settings_ = settings; }

    // 获取笔刷名称
    virtual const char* brushName() const = 0;

    // 强制结束当前笔触（跳过 FINALIZING）
    void forceEnd();

protected:
    BrushState state_ = BrushState::IDLE;
    Stroke currentStroke_;
    BrushSettings settings_;

    // 状态转换辅助
    void setState(BrushState newState);
    void transitionTo(BrushState newState);

    // 添加采样点到当前笔触
    void appendSample(const InputSample& sample);

    // 计算两点间的 BrushState（用于 BrushRenderer）
    BrushRenderState sampleToBrushState(const InputSample& sample, const InputSample& prev) const;
};