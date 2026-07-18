#version 450

// ============================================================================
// DabInstance - GPU 笔刷圆点实例数据
// 对应 CPU 侧的 BrushRenderState + BrushSettings 的 GPU 表示
// ============================================================================
struct DabInstance {
    vec2  center;          // 笔尖中心位置（像素坐标）
    float size;            // 笔刷大小（像素）
    float opacity;         // 笔刷不透明度 [0,1]
    float rotation;        // 笔尖旋转角度（弧度）
    float flattening;      // 笔尖压扁系数 [0,1]，1.0=正圆
    vec4  color;           // 笔刷颜色 RGBA（预乘 alpha 或线性空间）
    float textureScale;    // 纹理缩放比例
    float textureRotation; // 纹理自身旋转角度（弧度）
    uint  useTexture;      // 是否使用纹理笔刷（0=纯色椭圆，1=纹理）
    uint  blendMode;       // 混合模式索引：0=Normal, 1=Multiply, 2=Screen, ...
};

// ============================================================================
// Shader Storage Buffer Object - 每帧上传的 dab 实例数组
// ============================================================================
layout(set = 0, binding = 0) readonly buffer DabBuffer {
    DabInstance dabs[];
};

// ============================================================================
// Push Constants - 每 stroke 调用一次，不变的数据
// ============================================================================
layout(push_constant) uniform PushConstants {
    vec2  canvasSize;      // 画布像素尺寸（用于 NDC 转换）
    float globalOpacity;   // 全局不透明度乘数（来自图层或笔刷设置）
    int   numDabs;         // 本次渲染的 dab 数量（调试用，shader 中未使用）
} pc;

// ============================================================================
// 笔刷纹理（可选）
// ============================================================================
layout(set = 0, binding = 1) uniform sampler2D brushTexture;

// ============================================================================
// 顶点输出到片段着色器
// ============================================================================
layout(location = 0) out vec2  v_localPos;    // 椭圆局部坐标 [-1,1]，用于 coverage 计算
layout(location = 1) out vec4  v_color;       // 笔刷颜色
layout(location = 2) out float v_opacity;     // 最终不透明度
layout(location = 3) out flat uint v_blendMode; // 混合模式（flat 插值）
layout(location = 4) out vec2  v_texCoord;    // 纹理采样 UV
layout(location = 5) out flat uint v_useTexture; // 是否使用纹理（flat 插值）

// ============================================================================
// 全屏三角形/quad 顶点（无顶点输入，纯过程生成）
// 4 个顶点组成两个三角形：
//   0:(-1,-1)  1:(1,-1)  2:(-1,1)
//   1:(1,-1)   2:(-1,1)  3:(1,1)
// ============================================================================
const vec2 QUAD[4] = vec2[](
    vec2(-1.0, -1.0),   // 左下
    vec2( 1.0, -1.0),   // 右下
    vec2(-1.0,  1.0),   // 左上
    vec2( 1.0,  1.0)    // 右上
);

// ============================================================================
// 主函数
// ============================================================================
void main() {
    // -------------------------------------------------------------------------
    // 获取当前实例数据
    // gl_InstanceIndex: 当前实例在 SSBO 数组中的索引
    // gl_VertexIndex:   当前顶点的索引（0-3）
    // -------------------------------------------------------------------------
    DabInstance dab = dabs[gl_InstanceIndex];
    vec2 quadPos = QUAD[gl_VertexIndex];

    // -------------------------------------------------------------------------
    // 计算椭圆长短轴
    // 对应 CPU 代码: EllipseAxes(size, flattening, a, b)
    // a = size（长轴）
    // b = size * flattening（短轴，flattening=1.0 时为正圆）
    // -------------------------------------------------------------------------
    float a = dab.size;
    float b = dab.size * dab.flattening;

    // -------------------------------------------------------------------------
    // 将 quad 从 [-1,1] 单位空间映射到椭圆局部空间
    // quadPos.x * a → 椭圆长轴方向的半轴长度
    // quadPos.y * b → 椭圆短轴方向的半轴长度
    // -------------------------------------------------------------------------
    vec2 local = vec2(quadPos.x * a, quadPos.y * b);

    // -------------------------------------------------------------------------
    // 2D 旋转矩阵：将局部坐标旋转到世界空间
    // 对应 CPU 代码: rotate2D(dx, dy, -rotation, localX, localY)
    // 注意：这里旋转的是偏移量，不是反向旋转
    //
    // [ cosθ  -sinθ ] [ local.x ]
    // [ sinθ   cosθ ] [ local.y ]
    // -------------------------------------------------------------------------
    float c = cos(dab.rotation);
    float s = sin(dab.rotation);
    vec2 worldOffset = vec2(
        local.x * c - local.y * s,
        local.x * s + local.y * c
    );

    // -------------------------------------------------------------------------
    // 像素坐标 → NDC 坐标转换
    //
    // 步骤 1: (center + offset) 得到像素空间中的绝对位置
    // 步骤 2: / canvasSize 归一化到 [0,1]
    // 步骤 3: * 2.0 - 1.0 映射到 [-1,1]
    //
    // Vulkan NDC 坐标系：
    //   X: 左=-1, 右=+1
    //   Y: 上=-1, 下=+1（Y 向下）
    //   Z: 近=0, 远=1
    // -------------------------------------------------------------------------
    vec2 pixelPos = dab.center + worldOffset;
    vec2 ndcPos = (pixelPos / pc.canvasSize) * 2.0 - 1.0;
    ndcPos.y = -ndcPos.y;  // 翻转 Y 轴适配 Vulkan

    gl_Position = vec4(ndcPos, 0.0, 1.0);

    // -------------------------------------------------------------------------
    // 输出到片段着色器的 varyings
    // -------------------------------------------------------------------------
    // v_localPos: 保持 [-1,1] 范围，fragment 中用椭圆方程计算 coverage
    v_localPos = quadPos;

    // v_color: 直接传递笔刷颜色
    v_color = dab.color;

    // v_opacity: 笔刷自身 opacity * 全局 opacity
    // 注意：coverage 计算在 fragment 中，这里只乘全局系数
    v_opacity = dab.opacity * pc.globalOpacity;

    // v_blendMode: flat 插值，整个 dab 使用相同混合模式
    v_blendMode = dab.blendMode;

    // v_useTexture: flat 插值，整个 dab 统一是否采样纹理
    v_useTexture = dab.useTexture;

    // -------------------------------------------------------------------------
    // 纹理坐标计算
    // 步骤 1: quadPos [-1,1] → [0,1]
    // 步骤 2: 以 (0.5,0.5) 为中心旋转 textureRotation
    // 步骤 3: 按 textureScale 缩放
    // 对应 CPU 代码: tex.sampleTransformed(u, v, textureRotation, textureScale)
    // -------------------------------------------------------------------------
    vec2 uv = quadPos * 0.5 + 0.5;

    float tc = cos(dab.textureRotation);
    float ts = sin(dab.textureRotation);
    vec2 centered = uv - 0.5;
    vec2 rotated = vec2(
        centered.x * tc - centered.y * ts,
        centered.x * ts + centered.y * tc
    );
    v_texCoord = rotated / dab.textureScale + 0.5;
}