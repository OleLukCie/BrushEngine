#pragma once

#include "../base/base_types.h"
#include "../base/base_math.h"


// 颜色混合
enum class BlendMode {
    Normal, // 正常混合
    Multiply, // 正片叠底
    Add, // 加法混合
    Overlay, // 叠加
    Screen, // 滤色
    Darken, // 变暗
    Lighten, // 变亮
    ColorDodge, // 颜色减淡
    ColorBurn, //颜色加深
    HardLight, // 强光
    SoftLight, // 柔光
    Difference, // 差值
    Exclusion, // 排除
    Erase, // 擦除
};

// 标准Alpha混合
// @param src 源颜色
// @param dst 目标颜色
// @return 混合后的颜色
Color alphaBlend(const Color& src, const Color& dst);

// 预乘Alpha混合
Color premultipliedBlend(const Color& srcPremul, const Color& dst);

// 正常模式混合
Color blendNormal(const Color& src, const Color& dst);

// 正片叠底模式混合
Color blendMultiply(const Color& src, const Color& dst);

// 加法模式混合
Color blendAdd(const Color& src, const Color& dst);

// 叠加模式混合
Color blendOverlay(const Color& src, const Color& dst);

// 滤色模式混合
Color blendScreen(const Color& src, const Color& dst);

// 变暗模式混合
Color blendDarken(const Color& src, const Color& dst);

// 变亮模式混合
Color blendLighten(const Color& src, const Color& dst);

// 颜色减淡模式混合
Color blendColorDodge(const Color& src, const Color& dst);

// 颜色加深模式混合
Color blendColorBurn(const Color& src, const Color& dst);

// 强光模式混合
Color blendHardLight(const Color& src, const Color& dst);

// 柔光模式混合
Color blendSoftLight(const Color& src, const Color& dst);

// 差值模式混合
Color blendDifference(const Color& src, const Color& dst);

// 擦除模式混合
Color blendExclusion(const Color& src, const Color& dst);

// 擦除模式混合
Color blendErase(const Color& src, const Color& dst);

// 通用混合函数
Color blend(const Color& src, const Color& dst, BlendMode mode);

// 感知线性混合
Color blendPerceptual(const Color& src, const Color& dst, BlendMode mode, float gamma = 2.2f);


// 混合管线配置结构体
struct BlendPipeline {
    // 混合模式（默认正常）
    BlendMode mode = BlendMode::Normal;
    // 整体不透明度 （0=完全透明，1=完全不透明）
    float opacity = 1.0f;
    // 是否启用Gamma校正
    bool useGammaCorrection = false;
    // Gamma校正系数
    float gamma = 2.2f;

    // 应用混合管线配置计算最终颜色
    Color apply(const Color& src, const Color& dst) const;
};
