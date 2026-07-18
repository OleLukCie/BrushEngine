#version 450

// ============================================================================
// 图层合成 Fragment Shader
//
// 功能：将多个图层按顺序混合为最终图像
// 对应 CPU 代码: Canvas::composite() + compositeLayer()
// ============================================================================

// ============================================================================
// 输入：来自 vertex shader
// ============================================================================
layout(location = 0) in vec2 v_uv;

// ============================================================================
// 输出：最终合成像素
// ============================================================================
layout(location = 0) out vec4 outColor;

// ============================================================================
// 统一缓冲区：合成参数
//
// 对应 CPU 代码中 Canvas 的图层属性：
//   - opacity:  Layer::opacity()
//   - blendMode: Layer::blendMode()
//   - visible:  在 CPU 侧过滤，只传入可见图层
// ============================================================================
layout(set = 0, binding = 0) uniform CompositeUBO {
    // 图层数量（最多支持 16 层，与 texture array 大小匹配）
    int layerCount;

    // 画布背景色（对应 Canvas::backgroundColor_）
    vec4 backgroundColor;

    // 每图层参数：
    //   x = opacity    [0, 1]
    //   y = blendMode  0=Normal, 1=Multiply, 2=Screen, 3=Overlay, ...
    //   z = reserved
    //   w = reserved
    vec4 layerParams[16];
} ubo;

// ============================================================================
// 图层纹理数组
//
// 使用 sampler2D 数组，索引对应图层顺序（0=底层/背景，向上递增）
// 注意：GLSL 中 sampler2D 数组大小必须是编译时常量
// ============================================================================
layout(set = 0, binding = 1) uniform sampler2D layerTextures[16];

// ============================================================================
// 混合模式实现
// 对应 CPU 代码: BlendPipeline::apply(src, dst)
// ============================================================================

// Normal 模式：标准 alpha 混合（ painters algorithm ）
// 公式: result = src * src.a + dst * dst.a * (1 - src.a)
//       result.a = src.a + dst.a * (1 - src.a)
vec4 blendNormal(vec4 src, vec4 dst) {
    float outAlpha = src.a + dst.a * (1.0 - src.a);
    if (outAlpha < 0.0001) {
        return vec4(0.0, 0.0, 0.0, 0.0);
    }
    vec3 outRgb = (src.rgb * src.a + dst.rgb * dst.a * (1.0 - src.a)) / outAlpha;
    return vec4(outRgb, outAlpha);
}

// Multiply 模式
// 公式: result.rgb = src.rgb * dst.rgb
vec4 blendMultiply(vec4 src, vec4 dst) {
    vec3 multiplied = src.rgb * dst.rgb;
    float outAlpha = src.a + dst.a * (1.0 - src.a);
    vec3 outRgb = (multiplied * src.a + dst.rgb * dst.a * (1.0 - src.a));
    return vec4(outRgb, outAlpha);
}

// Screen 模式
// 公式: result.rgb = 1 - (1 - src.rgb) * (1 - dst.rgb)
vec4 blendScreen(vec4 src, vec4 dst) {
    vec3 screened = 1.0 - (1.0 - src.rgb) * (1.0 - dst.rgb);
    float outAlpha = src.a + dst.a * (1.0 - src.a);
    vec3 outRgb = (screened * src.a + dst.rgb * dst.a * (1.0 - src.a));
    return vec4(outRgb, outAlpha);
}

// Overlay 模式
// 公式: dst < 0.5 ? 2*src*dst : 1 - 2*(1-src)*(1-dst)
vec4 blendOverlay(vec4 src, vec4 dst) {
    vec3 overlay;
    for (int i = 0; i < 3; i++) {
        if (dst[i] < 0.5) {
            overlay[i] = 2.0 * src[i] * dst[i];
        } else {
            overlay[i] = 1.0 - 2.0 * (1.0 - src[i]) * (1.0 - dst[i]);
        }
    }
    float outAlpha = src.a + dst.a * (1.0 - src.a);
    vec3 outRgb = (overlay * src.a + dst.rgb * dst.a * (1.0 - src.a));
    return vec4(outRgb, outAlpha);
}

// 统一混合入口
vec4 blend(vec4 src, vec4 dst, uint mode) {
    switch(mode) {
        case 0u: return blendNormal(src, dst);
        case 1u: return blendMultiply(src, dst);
        case 2u: return blendScreen(src, dst);
        case 3u: return blendOverlay(src, dst);
        default: return blendNormal(src, dst);
    }
}

// ============================================================================
// 主函数
// ============================================================================
void main() {
    // -------------------------------------------------------------------------
    // Step 1: 从背景色开始
    // 对应 CPU 代码: Color result = backgroundColor;
    // -------------------------------------------------------------------------
    vec4 result = ubo.backgroundColor;

    // -------------------------------------------------------------------------
    // Step 2: 按顺序混合所有图层
    // 对应 CPU 代码:
    //   for (auto& layer : layers_) {
    //       if (!layer->visible()) continue;
    //       vec4 src = texture(layer);
    //       result = blend(src * opacity, result, blendMode);
    //   }
    //
    // 注意：图层顺序是 0=底层 → layerCount-1=顶层
    // 混合时从底层到顶层逐层叠加
    // -------------------------------------------------------------------------
    for (int i = 0; i < ubo.layerCount; i++) {
        // 采样当前图层像素
        vec4 src = texture(layerTextures[i], v_uv);

        // 应用图层不透明度
        // 对应 CPU 代码: src.a *= layer->opacity()
        float opacity = ubo.layerParams[i].x;
        src.a *= opacity;

        // 获取混合模式
        uint mode = uint(ubo.layerParams[i].y);

        // 混合到累积结果
        result = blend(src, result, mode);
    }

    // -------------------------------------------------------------------------
    // Step 3: 输出最终颜色
    // -------------------------------------------------------------------------
    outColor = result;
}