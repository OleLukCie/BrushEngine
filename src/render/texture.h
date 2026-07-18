#pragma once

#include "../base/base_types.h"
#include "render_target.h"
#include <vector>
#include <string>
#include <memory>

// 纹理包裹模式
enum class WrapMode {
    Clamp,  // 边界
    Repeat, // 重复平铺
    Mirror // 镜像重复
};

// 纹理过滤模式枚举
enum class FilterMode {
    Nearest, // 最近点采样
    Bilinear // 双线性采样
};

// 纹理类
class Texture {
public:
    Texture(); // 默认构造
    // 指定宽高构造
    Texture(int width, int height);
    // 宽高+填充色构造
    Texture(int width, int height, const Color& fillColor);

    // 从RGBA8数据创建纹理
    static Texture fromRGBA8(const std::vector<uint8_t>& data, int width, int height, float gamma = 2.2f);
    // 从灰度8位数据创建纹理
    static Texture fromGrayscale8(const std::vector<uint8_t>& data, int width, int height);
    // 从浮点数据创建纹理
    static Texture fromFloatData(const std::vector<float>& data, int width, int height, int channels = 1);

    int width() const { return width_; } // 获取宽度
    int height() const { return height_; } // 获取高度
    bool empty() const { return pixels_.empty(); } // 判断纹理是否为空

    // 获取指定坐标像素
    Color& pixel(int x, int y);
    const Color& pixel(int x, int y) const;

    // 安全获取像素
    Color pixelSafe(int x, int y) const;

    // 双线性采样
    Color sampleBilinear(float u, float v, WrapMode wrap = WrapMode::Clamp) const;

    // 最近点采样
    Color sampleNearest(float u, float v, WrapMode wrap = WrapMode::Clamp) const;

    // 通用采样
    Color sample(float u, float v, FilterMode filter = FilterMode::Bilinear, WrapMode wrap = WrapMode::Clamp) const;

    // 带旋转缩放的采样
    Color sampleTransformed(float u, float v, float rotation, float scale,
                           FilterMode filter = FilterMode::Bilinear,
                           WrapMode wrap = WrapMode::Clamp) const;

    // 获取像素数组
    std::vector<Color>& pixels() { return pixels_; }
    const std::vector<Color>& pixels() const { return pixels_; }

    // 生成普通噪声纹理
    static Texture generateNoise(int width, int height, float intensity = 1.0f);
    // 生成蓝噪声纹理
    static Texture generateBlueNoise(int width, int height, float intensity = 1.0f);
    // 生成线性渐变纹理
    static Texture generateGradient(int width, int height, const Color& c1, const Color& c2, bool horizontal = true);
    // 生成径向渐变纹理
    static Texture generateRadialGradient(int width, int height, const Color& center, const Color& edge);

    // 转为灰度浮点数据
    std::vector<float> toGrayscale() const;

    // 从灰度数据赋值
    void fromGrayscale(const std::vector<float>& gray);

private:
    int width_; // 纹理宽度
    int height_; // 纹理高度
    std::vector<Color> pixels_; // 像素数组

    // 计算纹理坐标包裹
    float wrapCoordinate(float coord, int size, WrapMode mode) const;
    // 计算索引包裹
    int wrapIndex(int idx, int size, WrapMode mode) const;
};

// 纹理采样器结构体
struct TextureSampler {
    // 绑定纹理
    const Texture* texture = nullptr;
    // 过滤模式
    FilterMode filter = FilterMode::Bilinear;
    // 包裹模式
    WrapMode wrap = WrapMode::Clamp;

    // 采样纹理
    Color sample(float u, float v) const;
    // 像素级采样
    Color sampleAtPixel(float x, float y) const;
    // 变换采样
    Color sampleTransformed(float u, float v, float rotation, float scale) const;
};
