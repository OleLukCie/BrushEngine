#include "render_target.h"
#include "../base/base_math.h"
#include <cstring>

// 创建指定宽高、默认透明黑色的渲染目标
CPURenderTarget::CPURenderTarget(int w, int h)
    : width_(w), height_(h), pixels_(w * h, Color(0, 0, 0, 0)) {}

// 创建指定宽高、用指定颜色填充的渲染目标
CPURenderTarget::CPURenderTarget(int w, int h, const Color& fillColor)
    : width_(w), height_(h), pixels_(w * h, fillColor) {}

// 清空画布为指定颜色
void CPURenderTarget::clear(const Color& color) {
    // 用std::fill批量填充所有像素为指定颜色，效率高于逐像素循环
    std::fill(pixels_.begin(), pixels_.end(), color);
}

// 获取像素
Color CPURenderTarget::getPixel(int x, int y) const {
    return pixels_[pixelIndex(x, y)];
}

// 设置像素
void CPURenderTarget::setPixel(int x, int y, const Color& color) {
    pixels_[pixelIndex(x, y)] = color;
}

// 安全获取像素：越界返回透明黑
Color CPURenderTarget::getPixelSafe(int x, int y) const {
    if (!inBounds(x, y)) return Color(0, 0, 0, 0);
    return getPixel(x, y);
}

// 安全设置像素：越界直接忽略
void CPURenderTarget::setPixelSafe(int x, int y, const Color& color) {
    if (!inBounds(x, y)) return;
    setPixel(x, y, color);
}

// 导入为标准RGBA8格式（uint8_t 0~255），带伽马校正
void CPURenderTarget::exportToRGBA8(std::vector<uint8_t>& out) const {
    out.resize(width_ * height_ * 4);
    const float gamma = 2.2f;
    for (int i = 0; i < width_ * height_; ++i) {
        const Color& c = pixels_[i];
        // 线性空间 -> 伽马校正 -> 转为0~255
        out[i * 4 + 0] = static_cast<uint8_t>(saturate(gammaCorrection(c.r, gamma)) * 255.0f);
        out[i * 4 + 1] = static_cast<uint8_t>(saturate(gammaCorrection(c.g, gamma)) * 255.0f);
        out[i * 4 + 2] = static_cast<uint8_t>(saturate(gammaCorrection(c.b, gamma)) * 255.0f);
        out[i * 4 + 3] = static_cast<uint8_t>(saturate(c.a) * 255.0f);
    }
}


// 从RGBA8数据导入，逆伽马校正转回线性空间
void CPURenderTarget::importFromRGBA8(const std::vector<uint8_t>& in, float gamma) {
    if (in.size() < static_cast<size_t>(width_ * height_ * 4)) return;
    for (int i = 0; i < width_ * height_; ++i) {
        float r = in[i * 4 + 0] / 255.0f;
        float g = in[i * 4 + 1] / 255.0f;
        float b = in[i * 4 + 2] / 255.0f;
        float a = in[i * 4 + 3] / 255.0f;

        // 逆伽马校正，存入线性空间
        pixels_[i] = Color(
            gammaCorrection(r, 1.0f / gamma),
            gammaCorrection(g, 1.0f / gamma),
            gammaCorrection(b, 1.0f / gamma),
            a
        );
    }
}
