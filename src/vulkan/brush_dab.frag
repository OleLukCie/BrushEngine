#version 450

// ============================================================================
// 输入：来自 vertex shader 的 varyings
// ============================================================================
layout(location = 0) in vec2  v_localPos;    // 椭圆局部坐标 [-1,1]
layout(location = 1) in vec4  v_color;       // 笔刷颜色
layout(location = 2) in float v_opacity;     // 笔刷不透明度（已乘全局 opacity）
layout(location = 3) in flat uint v_blendMode; // 混合模式索引
layout(location = 4) in vec2  v_texCoord;    // 纹理采样 UV
layout(location = 5) in flat uint v_useTexture; // 是否使用纹理笔刷

// ============================================================================
// 笔刷纹理采样器（与 vertex shader 中声明一致，binding=1）
// ============================================================================
layout(set = 0, binding = 1) uniform sampler2D brushTexture;

// ============================================================================
// 输出：写入当前渲染目标的像素颜色
// ============================================================================
layout(location = 0) out vec4 outColor;

// ============================================================================
// 椭圆覆盖值计算
// 对应 CPU 代码: ellipseCoverage(localX, localY, a, b)
//
// 参数 pos: 局部坐标，范围 [-1, 1]
// 参数 a:   长轴半轴长度（归一化为 1.0，因为 v_localPos 已经是归一化的椭圆空间）
// 参数 b:   短轴半轴长度（同上，归一化为 1.0）
//
// 椭圆方程: (x/a)^2 + (y/b)^2 = 1
// coverage = 1  当点在椭圆内部 (dist < 0)
// coverage → 0  当点接近椭圆边界 (0 ≤ dist ≤ 1)
// coverage = 0  当点在椭圆外部 (dist > 1)
// ============================================================================
float ellipseCoverage(vec2 pos, float a, float b) {
    float dist = (pos.x * pos.x) / (a * a) + (pos.y * pos.y) / (b * b);
    // smoothstep 提供 1 像素的边缘抗锯齿过渡
    return 1.0 - smoothstep(0.0, 1.0, dist);
}

// ============================================================================
// 纹理采样（带旋转和缩放的 UV 变换）
// 对应 CPU 代码: tex.sampleTransformed(u, v, textureRotation, textureScale)
//
// 注意：UV 变换已在 vertex shader 中完成，这里直接采样
// 但需要做 clamp 到 [0,1] 以避免纹理重复/镜像的 artifacts
// ============================================================================
vec4 sampleBrushTexture(vec2 uv) {
    // Clamp to edge 行为在 sampler 创建时已配置
    // 这里只需要确保 UV 在有效范围内
    vec2 clampedUV = clamp(uv, 0.0, 1.0);
    return texture(brushTexture, clampedUV);
}

// ============================================================================
// 混合模式实现
// 对应 CPU 代码: BlendPipeline::apply(src, dst)
//
// 所有混合公式都遵循"src-over-dst"的 painters algorithm：
// 结果 = blend(src.rgb, dst.rgb) * src.a + dst.rgb * dst.a * (1 - src.a)
//
// 但由于 Vulkan 硬件混合的限制，复杂模式（Multiply, Screen, Overlay）
// 无法在单个 render pass 中用固定管线混合实现。
// 这里提供两种策略：
//   1. 标准 Alpha 混合（Normal）：通过 VkPipelineColorBlendAttachmentState 硬件混合
//   2. 复杂模式：通过 subpass input / input attachment 读取 dst 像素，在 shader 中计算
//
// 当前实现假设使用策略 1（Normal 模式），复杂模式需要额外的 render pass 设计。
// ============================================================================

// Normal 模式：标准 alpha 混合
// 公式: result.rgb = src.rgb * src.a + dst.rgb * dst.a * (1 - src.a)
//       result.a  = src.a + dst.a * (1 - src.a)
// 这个公式由硬件混合管线自动处理，shader 只需输出 src（预乘 alpha）
vec4 blendNormal(vec4 src, vec4 dst) {
    return vec4(
        src.rgb * src.a + dst.rgb * dst.a * (1.0 - src.a),
        src.a + dst.a * (1.0 - src.a)
    );
}

// Multiply 模式
// 公式: result.rgb = src.rgb * dst.rgb
//       result.a   = src.a
vec4 blendMultiply(vec4 src, vec4 dst) {
    return vec4(
        src.rgb * dst.rgb * src.a + dst.rgb * dst.a * (1.0 - src.a),
        src.a + dst.a * (1.0 - src.a)
    );
}

// Screen 模式
// 公式: result.rgb = 1 - (1 - src.rgb) * (1 - dst.rgb)
vec4 blendScreen(vec4 src, vec4 dst) {
    vec3 screen = 1.0 - (1.0 - src.rgb) * (1.0 - dst.rgb);
    return vec4(
        screen * src.a + dst.rgb * dst.a * (1.0 - src.a),
        src.a + dst.a * (1.0 - src.a)
    );
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
    return vec4(
        overlay * src.a + dst.rgb * dst.a * (1.0 - src.a),
        src.a + dst.a * (1.0 - src.a)
    );
}

// 统一混合入口
// 注意：此函数需要 dst 像素，因此只能在支持 input attachment 的 render pass 中使用
// 对于标准 Normal 模式，硬件混合已足够，不需要调用此函数
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
    // Step 1: 计算椭圆覆盖值
    // v_localPos 范围 [-1, 1]，对应椭圆局部坐标系
    // a = b = 1.0 因为 v_localPos 已经是归一化的（quadPos 本身就是归一化单位圆）
    // -------------------------------------------------------------------------
    float coverage = ellipseCoverage(v_localPos, 1.0, 1.0);

    // 覆盖值过低时直接 discard，避免写入透明像素
    if (coverage < 0.001) {
        discard;
    }

    // -------------------------------------------------------------------------
    // Step 2: 计算源颜色（笔刷颜色）
    // -------------------------------------------------------------------------
    vec4 src = v_color;

    // -------------------------------------------------------------------------
    // Step 3: 应用纹理（如果使用纹理笔刷）
    // 对应 CPU 代码:
    //   Color texColor = tex.sampleTransformed(u, v, textureRotation, textureScale);
    //   Color src = color * texColor;
    //   src.a = coverage * opacity * texColor.a;
    //
    // 注意：这里颜色相乘是分量乘法（modulate）
    // -------------------------------------------------------------------------
    if (v_useTexture != 0u) {
        vec4 texColor = sampleBrushTexture(v_texCoord);
        src.rgb = src.rgb * texColor.rgb;  // 颜色调制
        src.a = src.a * texColor.a;        // Alpha 调制
    }

    // -------------------------------------------------------------------------
    // Step 4: 应用覆盖值和不透明度
    // 对应 CPU 代码: src.a = coverage * opacity
    //
    // v_opacity 已经在 vertex shader 中乘了全局 opacity：
    //   v_opacity = dab.opacity * pc.globalOpacity
    //
    // 最终 src alpha = coverage * v_opacity
    // -------------------------------------------------------------------------
    src.a = coverage * v_opacity;

    // -------------------------------------------------------------------------
    // Step 5: Alpha 预乘（Premultiplied Alpha）
    // Vulkan 标准 alpha 混合公式期望 src.rgb 已经是预乘的：
    //   result = src.rgb * 1 + dst.rgb * (1 - src.a)
    // 如果 src.rgb 没有预乘，需要在这里处理：
    //   src.rgb = src.rgb * src.a
    //
    // 注意：如果 CPU 侧传入的 color 已经是预乘的，这里可以跳过
    // 为安全起见，总是执行预乘（幂等：如果已经预乘，再乘一次 alpha 会错误）
    //
    // 决策：假设输入 color 是 straight alpha（非预乘），执行预乘
    // 如果后续发现颜色偏暗，说明输入已经是预乘的，需要移除这行
    // -------------------------------------------------------------------------
    src.rgb = src.rgb * src.a;

    // -------------------------------------------------------------------------
    // Step 6: 输出
    //
    // 策略 A（Normal 模式，推荐）：
    //   使用 VkPipelineColorBlendAttachmentState 配置为：
    //     blendEnable = VK_TRUE
    //     srcColorBlendFactor = VK_BLEND_FACTOR_ONE          (src.rgb 已预乘)
    //     dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
    //     colorBlendOp = VK_BLEND_OP_ADD
    //     srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE
    //     dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
    //     alphaBlendOp = VK_BLEND_OP_ADD
    //
    //   此时 outColor = src（预乘后的），硬件负责混合
    //
    // 策略 B（复杂混合模式）：
    //   需要使用 subpass input / input attachment 读取 dst 像素
    //   然后调用 blend(src, dst, v_blendMode)
    //   这需要特殊的 render pass 配置（见下方注释）
    //
    // 当前实现采用策略 A，输出预乘 alpha 的颜色
    // -------------------------------------------------------------------------
    outColor = src;

    // -------------------------------------------------------------------------
    // 复杂混合模式支持（注释掉的代码，需要 input attachment 支持）：
    //
    // layout(input_attachment_index = 0, set = 0, binding = 2) uniform subpassInput targetColor;
    //
    // void main() {
    //     ... // 上述步骤 1-5
    //     vec4 dst = subpassLoad(targetColor);
    //     outColor = blend(src, dst, v_blendMode);
    // }
    //
    // 对应的 render pass 需要：
    //   - pInputAttachments 指向当前 subpass 的 color attachment
    //   - 使用 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL 作为 input attachment layout
    //   - 管线禁用 blend（blendEnable = VK_FALSE），因为混合在 shader 中完成
    // -------------------------------------------------------------------------
}