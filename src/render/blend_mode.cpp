#include "blend_mode.h"
#include <cmath>

// 标准Alpha混合：源色覆盖目标色
Color alphaBlend(const Color& src, const Color& dst) {
    // 计算最终透明度
    float outA = src.a + dst.a * (1.0f - src.a);
    // 透明直接返回
    if (outA < 1e-6f) return Color(0, 0, 0, 0);

    // 计算混合后RGB通道
    float r = (src.r * src.a + dst.r * dst.a * (1.0f - src.a)) / outA;
    float g = (src.g * src.a + dst.g * dst.a * (1.0f - src.a)) / outA;
    float b = (src.b * src.a + dst.b * dst.a * (1.0f - src.a)) / outA;
    
    // 返回混合结果
    return Color(r, g, b, outA);
}


// 预乘Alpha混合：优化透明混合
Color premultipliedBlend(const Color& srcPremul, const Color& dst) {
    // 计算最终透明度
    float outA = srcPremul.a + dst.a * (1.0f - srcPremul.a);
    // 透明直接返回
    if (outA < 1e-6f) return Color(0, 0, 0, 0);
    // 预乘通道混合计算
    float r = (srcPremul.r + dst.r * dst.a * (1.0f - srcPremul.a)) / outA;
    float g = (srcPremul.g + dst.g * dst.a * (1.0f - srcPremul.a)) / outA;
    float b = (srcPremul.b + dst.b * dst.a * (1.0f - srcPremul.a)) / outA;
    
    // 返回混合结果
    return Color(r, g, b, outA);
}

// 正常混合：调用标准Alpha混合
Color blendNormal(const Color& src, const Color& dst) {
    return alphaBlend(src, dst);
}

// 正片叠底：颜色相乘后混合
Color blendMultiply(const Color& src, const Color& dst) {
    Color result;
    result.r = src.r * dst.r; // R通道相乘
    result.g = src.g * dst.g; // G通道相乘
    result.b = src.b * dst.b; // B通道相乘
    result.a = src.a; // 保留源透明度
    return alphaBlend(result, dst); // Alpha混合输出
}

// 相加混合：颜色值叠加
Color blendAdd(const Color& src, const Color& dst) {
    Color result;
    // 各通道相加并限制在0~1
    result.r = saturate(src.r + dst.r);
    result.g = saturate(src.g + dst.g);
    result.b = saturate(src.b + dst.b);
    result.a = saturate(src.a + dst.a);
    return result;
}

// 叠加混合：根据底色明暗增强对比
Color blendOverlay(const Color& src, const Color& dst) {
    // 叠加单通道计算逻辑
    auto overlayChannel = [](float s, float d) -> float {
        if (d < 0.5f) {
            return 2.0f * s * d; // 暗部
        } else {
            return 1.0f - 2.0f * (1.0f - s) * (1.0f - d); // 亮部
        }
    };
    Color result;
    result.r = overlayChannel(src.r, dst.r); // 计算R通道
    result.g = overlayChannel(src.g, dst.g); // 计算G通道
    result.b = overlayChannel(src.b, dst.b); // 计算B通道
    result.a = src.a;
    return alphaBlend(result, dst); // Alpha混合输出
}

// 滤色混合
Color blendScreen(const Color& src, const Color& dst) {
    Color result;
    result.r = 1.0f - (1.0f - src.r) * (1.0f - dst.r);
    result.g = 1.0f - (1.0f - src.g) * (1.0f - dst.g);
    result.b = 1.0f - (1.0f - src.b) * (1.0f - dst.b);
    result.a = src.a; // 保留源透明度
    return alphaBlend(result, dst);
}

// 变暗混合：取各通道最小值
Color blendDarken(const Color& src, const Color& dst) {
    Color result;
    result.r = std::min(src.r, dst.r);
    result.g = std::min(src.g, dst.g);
    result.b = std::min(src.b, dst.b);
    result.a = src.a;
    return alphaBlend(result, dst);
}

// 变亮混合：取各通道最大值
Color blendLighten(const Color& src, const Color& dst) {
    Color result;
    result.r = std::max(src.r, dst.r);
    result.g = std::max(src.g, dst.g);
    result.b = std::max(src.b, dst.b);
    result.a = src.a;
    return alphaBlend(result, dst);
}

// 颜色减淡
Color blendColorDodge(const Color& src, const Color& dst) {
    auto dodge = [](float s, float d) -> float {
        if (s >= 1.0f - 1e-6f) return 1.0f; // 源纯白返回1
        return saturate(d / (1.0f - s));
    };
    Color result;
    result.r = dodge(src.r, dst.r);
    result.g = dodge(src.g, dst.g);
    result.b = dodge(src.b, dst.b);
    result.a = src.a;
    return alphaBlend(result, dst);
}

// 颜色加深
Color blendColorBurn(const Color& src, const Color& dst) {
    auto burn = [](float s, float d) -> float {
        if (s <= 1e-6f) return 0.0f;
        return saturate(1.0f - (1.0f - d) / s);
    };
    Color result;
    result.r = burn(src.r, dst.r);
    result.g = burn(src.g, dst.g);
    result.b = burn(src.b, dst.b);
    result.a = src.a;
    return alphaBlend(result, dst);
}

// 强光混合
Color blendHardLight(const Color& src, const Color& dst) {
    auto hardLight = [](float s, float d) -> float {
        if (s < 0.5f) {
            return 2.0f * s * d;
        } else {
            return 1.0f - 2.0f * (1.0f - s) * (1.0f - d);
        }
    };
    Color result;
    result.r = hardLight(src.r, dst.r);
    result.g = hardLight(src.g, dst.g);
    result.b = hardLight(src.b, dst.b);
    result.a = src.a;
    return alphaBlend(result, dst);
}

// 柔光混合
Color blendSoftLight(const Color& src, const Color& dst) {
    auto softLight = [](float s, float d) -> float {
        if (s < 0.5f) {
            return d - (1.0f - 2.0f * s) * d * (1.0f - d);
        } else {
            float t;
            if (d < 0.25f) {
                t = ((16.0f * d - 12.0f) * d + 4.0f) * d;
            } else {
                t = std::sqrt(d);
            }
            return d + (2.0f * s - 1.0f) * (t - d);
        }
    };
    Color result;
    result.r = softLight(src.r, dst.r);
    result.g = softLight(src.g, dst.g);
    result.b = softLight(src.b, dst.b);
    result.a = src.a;
    return alphaBlend(result, dst);
}

// 差值混合：取绝对值差，产生反色效果
Color blendDifference(const Color& src, const Color& dst) {
    Color result;
    result.r = std::abs(src.r - dst.r);
    result.g = std::abs(src.g - dst.g);
    result.b = std::abs(src.b - dst.b);
    result.a = src.a;
    return alphaBlend(result, dst);
}

// 排除混合
Color blendExclusion(const Color& src, const Color& dst) {
    Color result;
    result.r = src.r + dst.r - 2.0f * src.r * dst.r;
    result.g = src.g + dst.g - 2.0f * src.g * dst.g;
    result.b = src.b + dst.b - 2.0f * src.b * dst.b;
    result.a = src.a;
    return alphaBlend(result, dst);
}

// 擦除混合：降低目标透明度
Color blendErase(const Color& src, const Color& dst) {
    Color result = dst; // 保留目标色
    result.a = saturate(dst.a - src.a); // 透明度相减
    return result;
}


// 混合总入口：根据模式选择对应算法
Color blend(const Color& src, const Color& dst, BlendMode mode) {
    switch (mode) {
        case BlendMode::Normal:     return blendNormal(src, dst);
        case BlendMode::Multiply:   return blendMultiply(src, dst);
        case BlendMode::Add:        return blendAdd(src, dst);
        case BlendMode::Overlay:    return blendOverlay(src, dst);
        case BlendMode::Screen:     return blendScreen(src, dst);
        case BlendMode::Darken:     return blendDarken(src, dst);
        case BlendMode::Lighten:    return blendLighten(src, dst);
        case BlendMode::ColorDodge: return blendColorDodge(src, dst);
        case BlendMode::ColorBurn:  return blendColorBurn(src, dst);
        case BlendMode::HardLight:  return blendHardLight(src, dst);
        case BlendMode::SoftLight:  return blendSoftLight(src, dst);
        case BlendMode::Difference: return blendDifference(src, dst);
        case BlendMode::Exclusion:  return blendExclusion(src, dst);
        case BlendMode::Erase:      return blendErase(src, dst);
        default:                    return blendNormal(src, dst);
    }
}


// 感知混合：伽马校正后混合
Color blendPerceptual(const Color& src, const Color& dst, BlendMode mode, float gamma) {
    Color srcP = gammaCorrectionRGB(src, gamma); // 源色伽马校正
    Color dstP = gammaCorrectionRGB(dst, gamma); // 目标色伽马校正
    Color resultP = blend(srcP, dstP, mode); // 执行混合
    return gammaCorrectionRGB(resultP, 1.0f / gamma); // 逆伽马恢复
}

// 混合管道应用
Color BlendPipeline::apply(const Color& src, const Color& dst) const {
    Color modSrc = src;
    modSrc.a *= opacity; // 应用透明度系数

    if (useGammaCorrection) {
        return blendPerceptual(modSrc, dst, mode, gamma); // 带伽马混合
    } else {
        return blend(modSrc, dst, mode); // 标准混合
    }
}
