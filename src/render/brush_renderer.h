#pragma once

#include "render_target.h"
#include "blend_mode.h"
#include "texture.h"
#include "../base/base_types.h"
#include "../base/base_math.h"
#include <vector>

// 画笔瞬时状态
struct BrushRenderState {
    Vec2 position; // 笔尖位置
    float pressure = 1.0f; // 压感值
    float velocity = 0.0f; // 画笔移动速度
    float direction = 0.0f; // 画笔移动方向
    float timestamp = 0.0f; // 时间戳
};

// 画笔全局配置
struct BrushSettings {
    Color color = Color(0, 0, 0, 1); // 颜色
    float baseSize = 10.0f; // 基础大小
    float minSize = 1.0f; // 最小尺寸
    float maxSize = 50.0f; //最大尺寸
    float opacity = 1.0f; // 不透明度

    float pressureSizePower = 0.5f; // 压感控制大小
    float pressureOpacity = true; // 是否启用压感控制透明度

    float velocityOpacityDecay = 0.5f; // 速度导致透明度衰减的系数
    bool useVelocityOpacity = false; // 是否启用速度越快越透明

    float flattening = 1.0f; // 笔尖压扁系数
    float rotation = 0.0f; // 笔尖固定旋转角度
    bool followDirection = true; // 笔尖是否跟随绘画方向自动旋转

    const Texture* texture = nullptr; // 画笔纹理
    float textureScale = 1.0f; // 纹理缩放比例
    float textureRotation = 0.0f; // 纹理自身旋转角度

    BlendMode blendMode = BlendMode::Normal; // 颜色混合模式

    float baseSpacing = 0.25f; // 笔尖圆点基础间距
    bool adaptiveSpacing = true; // 是否启用自适应间隔

    bool useStrokeFade = false; // 是否启用线条渐隐
    float fadeStart = 0.0f; // 渐隐起始位置
    float fadeEnd = 1.0f; // 渐隐结束位置
    float strokeProgress = 0.5f; // 当前线条绘制进度
};


// 画笔渲染器
class BrushRenderer {
public:
    BrushRenderer();
    ~BrushRenderer() = default;

    // 设置渲染目标
    void setTarget(RenderTarget* target);

    // 设置画笔配置
    void setSettings(const BrushSettings& settings) { settings_ = settings; }

    // 获取只读画笔配置
    const BrushSettings& settings() const { return settings_; }

    // 获取可修改画笔配置
    BrushSettings& settings() { return settings_; }

    // 渲染单个画笔圆点
    void renderDab(const Vec2& center, float size, float opacity,
                   float rotation, const Color& color);

    // 根据画笔瞬时状态，自动计算并渲染单个圆点
    void renderDab(const BrushRenderState& state);

    // 渲染两点之间的单条线条
    int renderStroke(const Vec2& from, const Vec2& to,
                     const BrushRenderState& fromState, const BrushRenderState& toState);

    // 渲染完整路径线条
    int renderStrokePath(const std::vector<BrushRenderState>& states);

    // 渲染平滑线条
    int renderSmoothStroke(const std::vector<Vec2>& points,
                           const std::vector<float>& pressures,
                           float smoothing = 0.5f);

private:
    RenderTarget* target_ = nullptr; // 渲染目标
    BrushSettings settings_; // 当前画笔配置

    // 根据压感计算最终画笔大小
    float computeSize(float pressure) const;

    // 计算透明度
    float computeOpacity(const BrushRenderState& state, float strokeT) const;

    // 计算笔尖旋转角度
    float computeRotation(const Vec2& direction) const;

    // 渲染纯色椭圆笔尖
    void renderEllipseDab(const Vec2& center, float a, float b,
                          float rotation, const Color& color, float opacity);

    // 渲染纹理笔尖
    void renderTexturedDab(const Vec2& center, float a, float b,
                           float rotation, const Color& color, float opacity);
};
