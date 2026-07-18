#pragma once

#include "../base/base_types.h"
#include <vector>
#include <memory>
#include <cstdint>

// 渲染目标抽象基类
class RenderTarget {
public:
    // 虚析构函数，默认实现
    virtual ~RenderTarget() = default;

    // 纯虚函数：获取像素数据
    virtual std::vector<Color>& pixels() = 0;
    virtual const std::vector<Color>& pixels() const = 0;

    // 获取宽度
    virtual int width() const = 0;
    // 获取高度
    virtual int height() const = 0;

    // 清空像素（默认透明）
    virtual void clear(const Color& color = Color(0, 0, 0, 0)) = 0;

    // 导出为RGBA8格式数据
    virtual void exportToRGBA8(std::vector<uint8_t>& out) const = 0;

    // 从RGBA8导入数据
    virtual void importFromRGBA8(const std::vector<uint8_t>& in, float gamma = 2.2f) = 0;

    // 获取指定坐标像素
    virtual Color getPixel(int x, int y) const = 0;
    // 设置指定坐标像素
    virtual void setPixel(int x, int y, const Color& color) = 0;
};

// CPU渲染目标类，继承自抽象基类
class CPURenderTarget : public RenderTarget {
public:
    // 指定宽高初始化
    CPURenderTarget(int w, int h);
    // 指定宽高和填充色初始化
    CPURenderTarget(int w, int h, const Color& fillColor);

    // 返回可修改像素数组
    std::vector<Color>& pixels() override { return pixels_; }
    // 返回只读像素数组
    const std::vector<Color>& pixels() const override { return pixels_; }

    // 返回宽度
    int width() const override { return width_; }
    // 返回高度
    int height() const override { return height_; }

    // 清空像素为指定颜色
    void clear(const Color& color = Color(0, 0, 0, 0)) override;

    // 导出为RGBA8字节数据
    void exportToRGBA8(std::vector<uint8_t>& out) const override;

    // 从RGBA8导入数据
    void importFromRGBA8(const std::vector<uint8_t>& in, float gamma = 2.2f) override;

    // 获取坐标像素颜色
    Color getPixel(int x, int y) const override;
    // 设置坐标像素颜色
    void setPixel(int x, int y, const Color& color) override;

    // 获取像素
    Color getPixelSafe(int x, int y) const;
    // 设置像素
    void setPixelSafe(int x, int y, const Color& color);

    // 计算像素一维索引
    int pixelIndex(int x, int y) const { return y * width_ + x; }
    bool inBounds(int x, int y) const { return x >= 0 && x < width_ && y >= 0 && y < height_; }

private:
    int width_; // 渲染目标宽度
    int height_; // 渲染目标高度
    std::vector<Color> pixels_; // 存储所有像素数据的数组
};