#include "base_math.h"

// Lambert 漫反射光照
// L = max(0, n · l) * diffuse
float lambertDiffuse(
    const Vec3& normal,   // 表面法线
    const Vec3& lightDir, // 光线方向
    float diffuse         // 漫反射强度系数
) {
    // 点积计算法线与光线的夹角
    float nDotL = normal.dot(lightDir);
    // max(0, ...)：背向光的地方强制变成0，不发光
    return std::max(0.0f, nDotL) * diffuse;
}

// RGB 漫反射
Color lambertDiffuseColor(
    const Vec3& normal,        // 表面法线
    const Vec3& lightDir,      // 光线方向
    const Color& diffuseColor  // 物体本身的漫反射颜色（固有色）
) {
    // 计算光照强度（0~1）
    float intensity = std::max(0.0f, normal.dot(lightDir));

    return Color(
        diffuseColor.r * intensity, // 红通道衰减
        diffuseColor.g * intensity, // 绿通道衰减
        diffuseColor.b * intensity, // 蓝通道衰减
        diffuseColor.a              // 透明度保持不变
    );
}

// Phong 高光光照
// spec = max(0, (2*(n·l)*n - l) · v)^shininess
float phongSpecular(
    const Vec3& normal,   // 单位法线
    const Vec3& lightDir, // 光线方向（单位向量）
    const Vec3& viewDir,  // 视线方向（单位向量）
    float shininess       // 高光硬度
) {
    // 计算法线与光线的点积
    float nDotL = normal.dot(lightDir);
    // 计算反射向量 r = 2*(n·l)*n - l
    Vec3 reflectDir = normal * (2.0f * nDotL) - lightDir;

    // To Do: Blinn-Phong
    // -> halfDir = normalize(lightDir + viewDir)
    // -> pow(max(0, dot(normal, halfDir)), shininess)

    // 反射向量与视线方向的点积
    // 越重合，看到的高光越强
    float rDotV = reflectDir.dot(viewDir);

    // 负数直接变0
    float spec = std::max(0.0f, rDotV);

    return std::pow(spec, shininess);
}

// 带高光颜色的 Phong 光照
Color phongLighting(
    const Vec3& normal,
    const Vec3& lightDir,
    const Vec3& viewDir,
    float shininess,
    const Color& specularColor // 高光颜色
) {
    // 计算高光强度
    float spec = phongSpecular(normal, lightDir, viewDir, shininess);
    
    // 最终高光颜色
    return Color(
        specularColor.r * spec,
        specularColor.g * spec,
        specularColor.b * spec,
        specularColor.a // 透明度不变
    );
}


Color phongLightingFull(
    const Vec3& normal,         // 表面法线
    const Vec3& lightDir,       // 光线方向
    const Vec3& viewDir,        // 视线方向
    const Color& diffuseColor,  // 物体固有色
    const Color& specularColor, // 高光颜色
    float shininess,            // 高光大小
    float ambientIntensity      // 高光强度
) {
    // 环境光
    // 让阴影区域不会纯黑，保证物体能看见
    Color ambient(
        diffuseColor.r * ambientIntensity,
        diffuseColor.g * ambientIntensity,
        diffuseColor.b * ambientIntensity,
        diffuseColor.a
    );

    // 漫反射
    // 物体基础明暗
    Color diffuse = lambertDiffuseColor(normal, lightDir, diffuseColor);

    // 高光
    // 光滑表面反光
    Color specular = phongLighting(
        normal, lightDir, viewDir, shininess, specularColor
    );

    // 合成
    // 限制最大值防止颜色过曝
    return Color(
        std::min(1.0f, ambient.r + diffuse.r + specular.r),
        std::min(1.0f, ambient.g + diffuse.g + specular.g),
        std::min(1.0f, ambient.b + diffuse.b + specular.b),
        diffuseColor.a
    );
}


// HSL 环形色相插值
// h_lerp = h1 + ((h2 - h1 + 0.5) mod 1 - 0.5) * t
// 色相是环形，不能直接线性插值，必须自动选择短弧插值，否则会反向绕圈变色
float hueLerp(float h1, float h2, float t) {
    // 色相归一化到[0,1)
    h1 = h1 - std::floor(h1);
    h2 = h2 - std::floor(h2);

    // 插值系数t限制在0~1
    t = std::max(0.0f, std::min(1.0f, t));

    // 计算环形最短路径差值
    float delta = h2 - h1;
    delta = delta + 0.5f;
    delta = delta - std::floor(delta);  // mod 1
    delta = delta - 0.5f;

    // 沿最短路径插值
    float result = h1 + delta * t;
    // 确保结果在 [0,1)
    return result - std::floor(result);
}


// 伽马亮度校正曲线
// out = in^gamma
// gamma > 1：变暗
// gamma < 1：变亮
float gammaCorrection(float value, float gamma) {
    // 限制在0~1之间
    value = std::max(0.0f, std::min(1.0f, value));

    // 防止值太小导致指数计算异常
    if (gamma < 1e-6f) return value;

    return std::pow(value, gamma);
}

// RGB三通道分别伽马校正
Color gammaCorrectionRGB(const Color& color, float gamma) {
    return Color(
        gammaCorrection(color.r, gamma),
        gammaCorrection(color.g, gamma),
        gammaCorrection(color.b, gamma),
        color.a // 透明度不参与伽马校正
    );
}



// Porter-Duff 标准 Alpha 混合
// 上层有透明度，不能完全覆盖下层；上层越不透明，下层显示越少
void PDAlphaBlend(
    const Color& src, // 源：上层画笔颜色
    const Color& dst, // 目标：下层画布已有颜色
    Color& out // 输出：混合后的最终颜色
) {
    // F_src = 1, F_dst = 1 - alpha_src
    float oneMinusSrcA = 1.0f - src.a;
    // 预乘Alpha颜色（适合计算）
    out.r = src.r * src.a + dst.r * dst.a * oneMinusSrcA;
    out.g = src.g * src.a + dst.g * dst.a * oneMinusSrcA;
    out.b = src.b * src.a + dst.b * dst.a * oneMinusSrcA;
    out.a = src.a + dst.a * oneMinusSrcA;

    // 转回非预乘颜色（适合画布显示）
    if (out.a > 1e-6f) {
        out.r /= out.a;
        out.g /= out.a;
        out.b /= out.a;
    }
}

// 正片叠底 (Multiply) 图层混合
void blendMultiply(
    const Color& src, // 上层颜色
    const Color& dst, // 下层颜色
    Color& out // 输出混合色
) {
    // 两个颜色相乘，越叠越深
    out.r = src.r * dst.r;
    out.g = src.g * dst.g;
    out.b = src.b * dst.b;

    // 不变暗透明度
    out.a = src.a + dst.a * (1.0f - src.a);
}